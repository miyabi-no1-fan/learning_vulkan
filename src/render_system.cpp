#include "render_system.hpp"

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "object.hpp"

struct PushConstant {
    glm::mat4x4 model_matrix;
    glm::mat4x4 normal_matrix;
};
static_assert(sizeof(PushConstant) <= 128, "The Vulkan spec only guaranteed 128 bytes of push constant");

RenderSystem::RenderSystem(Device& device, VkRenderPass renderpass, VkDescriptorSetLayout descriptor_layout) : device(device) {
    create_pipeline_layout(descriptor_layout);
    create_pipeline(renderpass);
}

RenderSystem::~RenderSystem() {
    vkDestroyPipelineLayout(device.device(), pipeline_layout, nullptr);
}

void RenderSystem::create_pipeline_layout(VkDescriptorSetLayout descriptor_layout) {
    VkPushConstantRange push_constant_range{
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(PushConstant),
    };

    std::vector<VkDescriptorSetLayout> descriptor_layouts{ descriptor_layout };

    VkPipelineLayoutCreateInfo pipeline_layout_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = static_cast<uint32_t>(descriptor_layouts.size()),
        .pSetLayouts = descriptor_layouts.data(),
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

void RenderSystem::render_objects(const FrameInfo& frame) {
    pipeline->bind(frame.command_buffer);

    vkCmdBindDescriptorSets(
        frame.command_buffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        pipeline_layout,
        0,
        1,
        &frame.global_descriptor_set,
        0,
        nullptr  //
    );

    for (auto&& kv : frame.objects) {
        auto&& object = kv.second;
        if (!object.model) continue;
        if (object.render) object.render(object, frame.time);

        PushConstant push{
            .model_matrix = object.model_matrix(),
            .normal_matrix = object.normal_matrix(),
        };

        vkCmdPushConstants(
            frame.command_buffer,
            pipeline_layout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(PushConstant),
            &push  //
        );

        object.model->bind(frame.command_buffer);
        object.model->draw(frame.command_buffer);
    }
}
