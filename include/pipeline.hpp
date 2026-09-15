#pragma once
#include <string>
#include <vector>

#include "device.hpp"

struct PipelineConfigInfo {
    PipelineConfigInfo(const PipelineConfigInfo&) = delete;
    PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

    VkPipelineViewportStateCreateInfo viewportInfo{};
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
    VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
    VkPipelineMultisampleStateCreateInfo multisampleInfo{};
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};

    std::vector<VkDynamicState> dynamicStateEnables{};
    VkPipelineDynamicStateCreateInfo dynamicStateInfo{};

    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;

    PipelineConfigInfo() {}
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
    Pipeline& operator=(const Pipeline&) = delete;

    static void default_pipeline_config_info(PipelineConfigInfo& config_info);

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
