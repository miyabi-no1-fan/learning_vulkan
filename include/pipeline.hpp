#pragma once
#include <cstdint>
#include <vulkan/vulkan.hpp>

#include "context.hpp"

struct PushConstant {
    std::uint32_t ignore;
};
static_assert(sizeof(PushConstant) <= 128, "The Vulkan spec only guaranteed 128 bytes of PushConstants");
static_assert(sizeof(PushConstant) % 4 == 0, "The Vulkan spec states: size must be a multiple of 4");

class ComputePipeline {
   public:
    vk::UniquePipeline pipeline;
    vk::UniquePipelineLayout pipeline_layout;
    vk::UniqueShaderModule shader_module;

    ComputePipeline(const VulkanContext& ctx, const vk::UniqueDescriptorSetLayout& descriptor_set_layout, const std::string& shader_file);
    void bind(const vk::UniqueCommandBuffer& cmd, const PushConstant& push_constants) const;
};
