#pragma once
#include <memory>

#include "device.hpp"
#include "frame_info.hpp"
#include "pipeline.hpp"

class RenderSystem {
   private:
    Device& device;
    std::unique_ptr<Pipeline> pipeline{};
    VkPipelineLayout pipeline_layout{};

   public:
    RenderSystem(Device& device, VkRenderPass renderpass, VkDescriptorSetLayout descriptor_layout);
    ~RenderSystem();

    RenderSystem(const RenderSystem&) = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;

    void render_objects(const FrameInfo& frame);

   private:
    static constexpr const char* VERTEX_SHADER_SRC = "shaders/dist/shader.vert.spv";
    static constexpr const char* FRAGMENT_SHADER_SRC = "shaders/dist/shader.frag.spv";

    void create_pipeline_layout(VkDescriptorSetLayout descriptor_layout);
    void create_pipeline(VkRenderPass renderpass);
};
