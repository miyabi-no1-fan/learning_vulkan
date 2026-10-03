#pragma once
#include <cstddef>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "buffer.hpp"
#include "context.hpp"

struct PushConstant {
    std::uint32_t count;
};
static_assert(sizeof(PushConstant) <= 128, "The Vulkan spec only guaranteed 128 bytes of PushConstants");

class ComputePipeline {
    const VulkanContext& ctx;

   public:
    std::vector<vk::DescriptorSetLayoutBinding> binding_layout;
    std::vector<const Buffer*> binding_buffers;

    vk::UniqueDescriptorSetLayout descriptor_set_layout;

    vk::UniquePipelineLayout pipeline_layout;
    vk::UniqueShaderModule shader_module;
    vk::UniquePipeline pipeline;

    vk::UniqueDescriptorPool descriptor_pool;
    vk::UniqueDescriptorSet descriptor_set;

    ComputePipeline(const VulkanContext& ctx) : ctx(ctx) {}
    ComputePipeline& add_binding(const Buffer* buffer, std::size_t count = 1);
    ComputePipeline& create_pipeline(const std::string& shader_file);

    // update with new binding buffers
    void update() const;

    void bind(const vk::UniqueCommandBuffer& cmd, const PushConstant& push_constants) const;
};
