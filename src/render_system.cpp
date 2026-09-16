#include "render_system.hpp"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

RenderSystem::RenderSystem(Device& device, VkRenderPass renderpass) : device(device) {
    create_pipeline_layout();
    create_pipeline(renderpass);
}

RenderSystem::~RenderSystem() {
    vkDestroyPipelineLayout(device.device(), pipeline_layout, nullptr);
}

void RenderSystem::create_pipeline_layout() {
    VkPushConstantRange push_constant_range{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(PushConstant),
    };

    VkPipelineLayoutCreateInfo pipeline_layout_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 0,
        .pSetLayouts = nullptr,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &push_constant_range,
    };

    {
        VkResult res = vkCreatePipelineLayout(device.device(), &pipeline_layout_info, nullptr, &pipeline_layout);
        if (res != VK_SUCCESS) {
            throw std::runtime_error("Can't create pipeline layout. Vulkan Error code: " + std::to_string(res));
        }
    }
}

void RenderSystem::create_pipeline(VkRenderPass renderpass) {
    assert(pipeline_layout != nullptr && "Cannot create pipeline without pipeline layout");

    PipelineConfigInfo pipeline_config{};
    Pipeline::default_pipeline_config_info(pipeline_config);

    pipeline_config.renderPass = renderpass;
    pipeline_config.pipelineLayout = pipeline_layout;

    pipeline = std::make_unique<Pipeline>(
        device,
        VERTEX_SHADER_SRC,
        FRAGMENT_SHADER_SRC,
        pipeline_config  //
    );
}

void RenderSystem::render_objects(VkCommandBuffer command_buffer, std::vector<std::unique_ptr<Object>>& objects) {
    pipeline->bind(command_buffer);
    for (auto&& object : objects) {
        object->render();
        vkCmdPushConstants(
            command_buffer,
            pipeline_layout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstant),
            &push_constant  //
        );
        object->model->bind(command_buffer);
        object->model->draw(command_buffer);
    }
}
