#include "renderer.hpp"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "context.hpp"
#include "swap_chain.hpp"
#include "vulkan/vulkan.hpp"

namespace aglea {

Renderer::Renderer(Window& window, const Context& ctx) : window(window), ctx(ctx) {
    create_swap_chain();
    create_command_buffers();
}

void Renderer::create_swap_chain() {
    auto extent = window.get_extent();
    while (extent.width == 0 || extent.height == 0) {
        extent = window.get_extent();
        window.wait_events();
    }
    ctx.device->waitIdle();

    std::unique_ptr<SwapChain> old_swap_chain = std::move(swap_chain);
    swap_chain = std::make_unique<SwapChain>(ctx, extent, old_swap_chain.get());
    if (old_swap_chain && !old_swap_chain->cmpeq_swap_formats(*swap_chain)) {
        throw std::runtime_error("Swap chain image format (or depth format) has changed");
    }
    old_swap_chain = nullptr;
}

void Renderer::create_command_buffers() {
    command_buffers = ctx.create_command_buffers(swap_chain->image_count());
}

void Renderer::begin_frame() {
    assert(!is_frame_in_progress() && "Can't call begin_frame() while frame is already in progress");

    vk::Result res = swap_chain->acquire_next_image(current_image_index);
    if (res == vk::Result::eErrorOutOfDateKHR) {
        create_swap_chain();
        return;
    }
    if (res != vk::Result::eSuccess && res != vk::Result::eSuboptimalKHR) {
        throw std::runtime_error("Can't accquire swap chain image. Vulkan Error code: " + std::to_string((int)res));
    }
    is_frame_started = true;

    get_current_command_buffer()->begin(vk::CommandBufferBeginInfo());
}

void Renderer::end_frame() {
    assert(is_frame_in_progress() && "Can't call end_frame() while frame is not in progress");

    get_current_command_buffer()->end();

    auto res = swap_chain->submit_command_buffer(get_current_command_buffer(), current_image_index);
    if (res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR || window.was_window_resized()) {
        window.reset_window_resized_flag();
        create_swap_chain();
    } else if (res != vk::Result::eSuccess) {
        throw std::runtime_error("Can't present swap chain image. Vulkan Error code: " + std::to_string((int)res));
    }

    is_frame_started = false;
    current_frame_index++;
    current_frame_index %= SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::begin_renderpass() {
    assert(is_frame_in_progress() && "Can't call begin_renderpass() while frame is not in progress");

    vk::ClearValue clear_values[2];
    clear_values[0].color = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 0.0f);
    clear_values[1].depthStencil = vk::ClearDepthStencilValue(1.0f, 0);

    get_current_command_buffer()->beginRenderPass(
        vk::RenderPassBeginInfo(
            *swap_chain->get_render_pass(),
            *swap_chain->get_frame_buffer(current_image_index),
            vk::Rect2D({ 0, 0 }, swap_chain->get_swap_chain_extent()),
            clear_values),
        vk::SubpassContents::eInline);

    auto extent = swap_chain->get_swap_chain_extent();
    vk::Viewport viewport(
        0.0f,
        0.0f,
        static_cast<float>(extent.width),
        static_cast<float>(extent.height),
        0.0f,
        1.0f);
    vk::Rect2D scissor = { { 0, 0 }, extent };
    get_current_command_buffer()->setViewport(0, viewport);
    get_current_command_buffer()->setScissor(0, scissor);
}

void Renderer::end_renderpass() {
    assert(is_frame_in_progress() && "Can't call end_renderpass() while frame is not in progress");
    get_current_command_buffer()->endRenderPass();
}

}  // namespace aglea