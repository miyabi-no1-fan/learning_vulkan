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

ComputePipeline::ComputePipeline(const VulkanContext& ctx, const vk::UniqueDescriptorSetLayout& descriptor_set_layout, const std::string& shader_file) {
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
}

void ComputePipeline::bind(const vk::UniqueCommandBuffer& cmd, const PushConstant& push_constants) const {
    cmd->bindPipeline(vk::PipelineBindPoint::eCompute, *pipeline);
    cmd->pushConstants<PushConstant>(
        *pipeline_layout,
        vk::ShaderStageFlagBits::eCompute,
        0,
        push_constants  //
    );
}
