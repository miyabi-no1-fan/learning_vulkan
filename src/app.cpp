#include "app.hpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "model.hpp"

App::App() {
    create_pipeline_layout();
    create_swap_chain();
    create_pipeline();
    create_command_buffers();
    load_models();
}

App::~App() {
    vkDestroyPipelineLayout(device.device(), pipeline_layout, nullptr);
}

void App::run() {
    while (!window.should_close()) {
        window.poll_events();
        draw_frame();
    }

    auto _ = vkDeviceWaitIdle(device.device());
}

void App::load_models() {
    size_t image_count = 0;

    if (swap_chain) {
        image_count = swap_chain->imageCount();
    } else if (!command_buffers.empty()) {
        image_count = command_buffers.size();
    } else {
        assert(false && "Can't load models without swap chain or command buffers");
    }

    models.resize(image_count);

    state.vertices = {
        { { -0.5f, 0.5f }, { 1.0, 0.0, 0.0 } },
        { { 0.0f, -0.5f }, { 0.0, 1.0, 0.0 } },
        { { 0.5f, 0.5f }, { 0.0, 0.0, 1.0 } },
    };

    for (auto&& model : models) {
        model = std::make_unique<Model>(device, state.vertices);
    }
}

void App::create_pipeline_layout() {
    VkPipelineLayoutCreateInfo pipeline_layout_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr,
    };

    {
        VkResult res = vkCreatePipelineLayout(device.device(), &pipeline_layout_info, nullptr, &pipeline_layout);
        if (res != VK_SUCCESS) {
            throw std::runtime_error("Can't create pipeline layout. Vulkan Error code: " + std::to_string(res));
        }
    }
}

void App::create_pipeline() {
    assert(swap_chain != nullptr && "Cannot create pipeline without swap chain");
    assert(pipeline_layout != nullptr && "Cannot create pipeline without pipeline layout");

    PipelineConfigInfo pipeline_config{};
    Pipeline::default_pipeline_config_info(pipeline_config);

    pipeline_config.renderPass = swap_chain->getRenderPass();
    pipeline_config.pipelineLayout = pipeline_layout;

    pipeline = std::make_unique<Pipeline>(
        device,
        VERTEX_SHADER_SRC,
        FRAGMENT_SHADER_SRC,
        pipeline_config  //
    );
}

void App::create_swap_chain() {
    swap_chain = std::make_unique<SwapChain>(device, window.get_extent(), nullptr);
}

void App::recreate_swap_chain() {
    auto extent = window.get_extent();
    while (extent.width == 0 || extent.height == 0) {
        extent = window.get_extent();
        glfwWaitEvents();
    }
    auto _ = vkDeviceWaitIdle(device.device());

    swap_chain = std::make_unique<SwapChain>(device, extent, std::move(swap_chain));
    if (swap_chain->imageCount() != command_buffers.size()) {
        free_command_buffers();
        create_command_buffers();
    }

    create_pipeline();
}

void App::create_command_buffers() {
    command_buffers.resize(swap_chain->imageCount(), {});

    VkCommandBufferAllocateInfo alloc_info{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = device.getCommandPool(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t>(command_buffers.size()),
    };

    {
        VkResult res = vkAllocateCommandBuffers(device.device(), &alloc_info, command_buffers.data());
        if (res != VK_SUCCESS) {
            throw std::runtime_error("Can't allocate command buffers. Vulkan Error code: " + std::to_string(res));
        }
    }
}

void App::free_command_buffers() {
    vkFreeCommandBuffers(device.device(), device.getCommandPool(), static_cast<uint32_t>(command_buffers.size()), command_buffers.data());
    command_buffers.clear();
}

void App::record_command_buffer(size_t image_index) {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    {
        VkResult res = vkBeginCommandBuffer(command_buffers[image_index], &begin_info);
        if (res != VK_SUCCESS) {
            throw std::runtime_error("Can't begin recording command buffer. Vulkan Error code: " + std::to_string(res));
        }
    }

    std::array<VkClearValue, 2> clear_values{};
    clear_values[0].color = { { 0.1f, 0.1f, 0.1f, 1.0f } };
    clear_values[1].depthStencil = { 1.0f, 0 };

    VkRenderPassBeginInfo render_pass_info{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = swap_chain->getRenderPass(),
        .framebuffer = swap_chain->getFrameBuffer(image_index),
        .renderArea = {
            .offset = { 0, 0 },
            .extent = swap_chain->getSwapChainExtent(),
        },
        .clearValueCount = static_cast<uint32_t>(clear_values.size()),
        .pClearValues = clear_values.data(),
    };

    vkCmdBeginRenderPass(command_buffers[image_index], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

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
    vkCmdSetViewport(command_buffers[image_index], 0, 1, &viewport);
    vkCmdSetScissor(command_buffers[image_index], 0, 1, &scissor);

    pipeline->bind(command_buffers[image_index]);
    models[image_index]->bind(command_buffers[image_index]);
    models[image_index]->draw(command_buffers[image_index]);

    vkCmdEndRenderPass(command_buffers[image_index]);

    {
        VkResult res = vkEndCommandBuffer(command_buffers[image_index]);
        if (res != VK_SUCCESS) {
            throw std::runtime_error("Can't end record command buffer. Vulkan Error code: " + std::to_string(res));
        }
    }
}

void App::draw_frame() {
    uint32_t image_index{};
    {
        auto res = swap_chain->acquireNextImage(&image_index);

        if (res == VK_ERROR_OUT_OF_DATE_KHR) {
            recreate_swap_chain();
            return;
        }

        if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Can't accquire swap chain image. Vulkan Error code: " + std::to_string(res));
        }
    }
    update_model(image_index);
    record_command_buffer(image_index);
    {
        auto res = swap_chain->submitCommandBuffers(&command_buffers[image_index], &image_index);

        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || window.was_window_resized()) {
            window.reset_window_resized_flag();
            recreate_swap_chain();
            return;
        }

        if (res != VK_SUCCESS) {
            throw std::runtime_error("Can't present swap chain image. Vulkan Error code: " + std::to_string(res));
        }
    }
}

void App::update_model(size_t image_index) {
    if (++state.current_frame % FPS == 0) {
        std::vector<Model::Vertex> new_vertices{};
        new_vertices.reserve(state.vertices.size() * 3);

        auto v = state.vertices.cbegin();
        while (v != state.vertices.cend()) {
            auto a0 = v++.base();
            auto a1 = v++.base();
            auto a2 = v++.base();

            const Model::Vertex a01 = { (a0->position + a1->position) / 2.0f, (a0->color + a1->color) / 2.0f };
            const Model::Vertex a02 = { (a0->position + a2->position) / 2.0f, (a0->color + a2->color) / 2.0f };
            const Model::Vertex a12 = { (a1->position + a2->position) / 2.0f, (a1->color + a2->color) / 2.0f };

            new_vertices.push_back(*a0);
            new_vertices.push_back(a01);
            new_vertices.push_back(a02);

            new_vertices.push_back(*a1);
            new_vertices.push_back(a01);
            new_vertices.push_back(a12);

            new_vertices.push_back(*a2);
            new_vertices.push_back(a02);
            new_vertices.push_back(a12);
        }

        state.vertices = std::move(new_vertices);
    }

    models[image_index] = std::make_unique<Model>(device, state.vertices);
}
