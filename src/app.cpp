#include "app.hpp"

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "model.hpp"

App::App() {
    load_models();
    create_pipeline_layout();
    create_pipeline();
    create_command_buffers();
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
    std::vector<Model::Vertex> vertices = {
        { { -0.5f, 0.5f }, { 1.0, 0.0, 0.0 } },
        { { 0.0f, -0.5f }, { 0.0, 1.0, 0.0 } },
        { { 0.5f, 0.5f }, { 0.0, 0.0, 1.0 } },
    };
    model = std::make_unique<Model>(device, vertices);
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
    auto pipeline_config = Pipeline::default_config_info(swap_chain.width(), swap_chain.height());

    pipeline_config.renderPass = swap_chain.getRenderPass();
    pipeline_config.pipelineLayout = pipeline_layout;

    pipeline = std::make_unique<Pipeline>(
        device,
        VERTEX_SHADER_SRC,
        FRAGMENT_SHADER_SRC,
        pipeline_config  //
    );
}

void App::create_command_buffers() {
    command_buffers.resize(swap_chain.imageCount(), {});

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

    for (size_t i = 0; i < command_buffers.size(); i++) {
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        {
            VkResult res = vkBeginCommandBuffer(command_buffers[i], &begin_info);
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
            .renderPass = swap_chain.getRenderPass(),
            .framebuffer = swap_chain.getFrameBuffer(i),
            .renderArea = {
                .offset = { 0, 0 },
                .extent = swap_chain.getSwapChainExtent(),
            },
            .clearValueCount = static_cast<uint32_t>(clear_values.size()),
            .pClearValues = clear_values.data(),
        };

        vkCmdBeginRenderPass(command_buffers[i], &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

        pipeline->bind(command_buffers[i]);
        model->bind(command_buffers[i]);
        model->draw(command_buffers[i]);

        vkCmdEndRenderPass(command_buffers[i]);

        {
            VkResult res = vkEndCommandBuffer(command_buffers[i]);
            if (res != VK_SUCCESS) {
                throw std::runtime_error("Can't end record command buffer. Vulkan Error code: " + std::to_string(res));
            }
        }
    }
}

void App::draw_frame() {
    uint32_t image_index{};
    {
        auto res = swap_chain.acquireNextImage(&image_index);
        if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("Can't accquire swap chain image. Vulkan Error code: " + std::to_string(res));
        }
    }
    {
        auto res = swap_chain.submitCommandBuffers(&command_buffers[image_index], &image_index);
        if (res != VK_SUCCESS) {
            throw std::runtime_error("Can't present swap chain image. Vulkan Error code: " + std::to_string(res));
        }
    }
}
