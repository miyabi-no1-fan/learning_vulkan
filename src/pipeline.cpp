#include "pipeline.hpp"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <vector>
#include <vulkan/vulkan.hpp>

static std::vector<std::uint32_t> read_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Can't open file: " + path);
    }

    std::size_t len = static_cast<std::size_t>(file.tellg());
    if (len == 0 || len % sizeof(std::uint32_t) != 0) {
        throw std::runtime_error("Invalid SPIR-V file size: " + path);
    }

    std::vector<std::uint32_t> buf(len / sizeof(std::uint32_t));
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(len));
    return buf;
}

ComputePipeline& ComputePipeline::add_binding(const Buffer* buffer, std::size_t count) {
    for (std::size_t i = 0; i < count; i++) {
        binding_buffers.push_back(buffer + i);
        binding_layout.push_back(vk::DescriptorSetLayoutBinding(
            binding_layout.size(),
            vk::DescriptorType::eStorageBuffer,
            1,
            vk::ShaderStageFlagBits::eCompute));
    }
    return *this;
}

ComputePipeline& ComputePipeline::create_pipeline(const std::string& shader_file) {
    descriptor_set_layout =
        ctx.device->createDescriptorSetLayoutUnique(
            vk::DescriptorSetLayoutCreateInfo({}, binding_layout));

    vk::PushConstantRange push_constant_range(vk::ShaderStageFlagBits::eCompute, 0, sizeof(PushConstant));
    pipeline_layout =
        ctx.device->createPipelineLayoutUnique(
            vk::PipelineLayoutCreateInfo({}, *descriptor_set_layout, push_constant_range));

    std::vector<uint32_t> shader_code = read_file(shader_file);
    shader_module = ctx.device->createShaderModuleUnique(
        vk::ShaderModuleCreateInfo({}, shader_code.size() * sizeof(uint32_t), shader_code.data()));

    pipeline =
        ctx.device->createComputePipelineUnique(
                      {},
                      vk::ComputePipelineCreateInfo(
                          {},
                          vk::PipelineShaderStageCreateInfo(
                              {},
                              vk::ShaderStageFlagBits::eCompute,
                              *shader_module,
                              "main"),
                          *pipeline_layout))
            .value;

    vk::DescriptorPoolSize descriptor_pool_size(vk::DescriptorType::eStorageBuffer, binding_layout.size());
    descriptor_pool =
        ctx.device->createDescriptorPoolUnique(
            vk::DescriptorPoolCreateInfo(
                vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, descriptor_pool_size));

    std::vector<vk::UniqueDescriptorSet> descriptor_sets =
        ctx.device->allocateDescriptorSetsUnique(vk::DescriptorSetAllocateInfo(*descriptor_pool, *descriptor_set_layout));

    descriptor_set = std::move(descriptor_sets[0]);

    return *this;
}

void ComputePipeline::update() const {
    std::vector<vk::DescriptorBufferInfo> infos(binding_buffers.size(), vk::DescriptorBufferInfo());
    std::vector<vk::WriteDescriptorSet> writes(binding_buffers.size(), vk::WriteDescriptorSet());

    for (std::size_t i = 0; i < binding_buffers.size(); i++) {
        infos[i] = vk::DescriptorBufferInfo(*(binding_buffers[i]->buffer), 0, vk::WholeSize);
        writes[i] = vk::WriteDescriptorSet(
            *descriptor_set,
            binding_layout[i].binding,
            0,
            1,
            binding_layout[i].descriptorType,
            nullptr,
            &infos[i]);
    }

    ctx.device->updateDescriptorSets(writes, {});
}

void ComputePipeline::bind(const vk::UniqueCommandBuffer& cmd, const PushConstant& push_constants) const {
    cmd->bindPipeline(vk::PipelineBindPoint::eCompute, *pipeline);
    cmd->bindDescriptorSets(
        vk::PipelineBindPoint::eCompute,
        *pipeline_layout,
        0,
        *descriptor_set,
        {}  //
    );
    cmd->pushConstants<PushConstant>(
        *pipeline_layout,
        vk::ShaderStageFlagBits::eCompute,
        0,
        push_constants  //
    );
}
