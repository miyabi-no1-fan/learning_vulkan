#include "renderer.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "swap_chain.hpp"

Renderer::Renderer(Window& window, Device& device) : window(window), device(device) {
    create_swap_chain();
    create_command_buffers();
}

Renderer::~Renderer() {
    free_command_buffers();
}

void Renderer::create_swap_chain() {
    auto extent = window.get_extent();
    while (extent.width == 0 || extent.height == 0) {
        extent = window.get_extent();
        glfwWaitEvents();
    }
    auto _ = vkDeviceWaitIdle(device.device());

    std::shared_ptr<SwapChain> old_swap_chain = std::move(swap_chain);
    swap_chain = std::make_unique<SwapChain>(device, extent, old_swap_chain);
    if (old_swap_chain && !old_swap_chain->compareSwapFormats(*swap_chain.get())) {
        throw std::runtime_error("Swap chain image format (or depth format) has changed");
    }
    old_swap_chain = nullptr;
}

void Renderer::create_command_buffers() {
    command_buffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT, {});

    VkCommandBufferAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = device.getCommandPool(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(command_buffers.size()),
    };

    VkResult res = vkAllocateCommandBuffers(device.device(), &alloc_info, command_buffers.data());
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Can't allocate command buffers. Vulkan Error code: " + std::to_string(res));
    }
}

void Renderer::free_command_buffers() {
    vkFreeCommandBuffers(device.device(), device.getCommandPool(), static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
    command_buffers.clear();
}

void Renderer::begin_frame() {
    assert(!is_frame_in_progress() && "Can't call begin_frame() while frame is already in progress");

    VkResult res = swap_chain->acquireNextImage(&current_image_index);
    if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        create_swap_chain();
        return;
    }
    if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Can't accquire swap chain image. Vulkan Error code: " + std::to_string(res));
    }
    is_frame_started = true;

    auto command_buffer = get_current_command_buffer();

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    res = vkBeginCommandBuffer(command_buffer, &begin_info);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Can't begin recording command buffer. Vulkan Error code: " + std::to_string(res));
    }
}

void Renderer::end_frame() {
    assert(is_frame_in_progress() && "Can't call end_frame() while frame is not in progress");

    auto command_buffer = get_current_command_buffer();

    VkResult res = vkEndCommandBuffer(command_buffer);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Can't end record command buffer. Vulkan Error code: " + std::to_string(res));
    }

    res = swap_chain->submitCommandBuffers(&command_buffer, &current_image_index);
    if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || window.was_window_resized()) {
        window.reset_window_resized_flag();
        create_swap_chain();
    } else if (res != VK_SUCCESS) {
        throw std::runtime_error("Can't present swap chain image. Vulkan Error code: " + std::to_string(res));
    }

    is_frame_started = false;
    ++current_frame_index;
    current_frame_index %= SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::begin_renderpass() {
    assert(is_frame_in_progress() && "Can't call begin_renderpass() while frame is not in progress");

    auto command_buffer = get_current_command_buffer();

    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
    clear_values[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo render_pass_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = swap_chain->getRenderPass(),
        .framebuffer = swap_chain->getFrameBuffer(current_image_index),
        .renderArea = {
            .offset = { 0, 0 },
            .extent = swap_chain->getSwapChainExtent(),
        },
        .clearValueCount = static_cast<uint32_t>(clear_values.size()),
        .pClearValues = clear_values.data(),
    };
    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    auto extent = swap_chain->getSwapChainExtent();
    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(extent.width),
        .height = static_cast<float>(extent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    VkRect2D scissor = { { 0, 0 }, extent };
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

void Renderer::end_renderpass() {
    assert(is_frame_in_progress() && "Can't call end_renderpass() while frame is not in progress");
    auto command_buffer = get_current_command_buffer();
    vkCmdEndRenderPass(command_buffer);
}
