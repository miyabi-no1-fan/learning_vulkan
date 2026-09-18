#pragma once
#include <memory>
#include <vector>

#include "device.hpp"
#include "object.hpp"
#include "pipeline.hpp"

class RenderSystem {
   private:
    Device& device;
    std::unique_ptr<Pipeline> pipeline{};
    VkPipelineLayout pipeline_layout{};

   public:
    RenderSystem(Device& device, VkRenderPass renderpass);
    ~RenderSystem();

    RenderSystem(const RenderSystem&) = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;

    void render_objects(VkCommandBuffer command_buffer, std::vector<Object>& objects);

   private:
    static constexpr const char* VERTEX_SHADER_SRC = "shaders/dist/shader.vert.spv";
    static constexpr const char* FRAGMENT_SHADER_SRC = "shaders/dist/shader.frag.spv";

    void create_pipeline_layout();
    void create_pipeline(VkRenderPass renderpass);
};
