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

    struct PushConstant {
        alignas(16) glm::mat2x2 transform{ 1.0f };
        alignas(8) glm::vec2 shift{};

        void next_transform(glm::mat2x2 mat);
        void next_shift(glm::vec2 offset);
    } push_constant{};
    static_assert(sizeof(PushConstant) <= 128, "Only 128 bytes of memory guaranteed to be available for push constants");

   public:
    RenderSystem(Device& device, VkRenderPass renderpass);
    ~RenderSystem();

    RenderSystem(const RenderSystem&) = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;

    void render_objects(VkCommandBuffer command_buffer, std::vector<std::unique_ptr<Object>>& objects);

   private:
    static constexpr const char* VERTEX_SHADER_SRC = "shaders/dist/shader.vert.spv";
    static constexpr const char* FRAGMENT_SHADER_SRC = "shaders/dist/shader.frag.spv";

    void create_pipeline_layout();
    void create_pipeline(VkRenderPass renderpass);
};
