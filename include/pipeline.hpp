#pragma once
#include <string>
#include <vector>

#include "device.hpp"

struct PipelineConfigInfo {
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

class Pipeline {
   public:
    Pipeline(
        Device& device,
        const std::string& vertex_shader_path,
        const std::string& fragment_shader_path,
        const PipelineConfigInfo& config_info  //
    );
    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    void operator=(const Pipeline&) = delete;

    static PipelineConfigInfo default_config_info(uint32_t width, uint32_t height);

    void bind(VkCommandBuffer command_buffer);

   private:
    Device& device;
    VkPipeline graphics_pipeline;
    VkShaderModule vertex_shader_module;
    VkShaderModule fragment_shader_module;

    static std::vector<char> read_file(const std::string& path);
    void create_graphics_pipeline(
        const std::string& vertex_shader_path,
        const std::string& fragment_shader_path,
        const PipelineConfigInfo& config_info  //
    );
    void create_shader_module(const std::vector<char>& code, VkShaderModule* shader_module);
};
