#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "buffer.hpp"
#include "context.hpp"
#include "descriptors.hpp"
#include "image.hpp"
#include "matrix.hpp"
#include "pipeline.hpp"
#include "swap_chain.hpp"
#include "window.hpp"

namespace aglea {

struct GlobalUBO {
    vec2 scale;
};

class App {
    //
   public:
    static constexpr std::uint32_t DEFAULT_WIDTH = 1920;
    static constexpr std::uint32_t DEFAULT_HEIGHT = 1080;
    static constexpr const char* NAME = "Aglea";

   private:
    std::uint32_t width;  // the video's width and height
    std::uint32_t height;

    Window window{ DEFAULT_WIDTH, DEFAULT_HEIGHT, NAME, 1.0 / 60.0 };
    Context ctx{ window };

    std::unique_ptr<SwapChain> swap_chain{};
    std::vector<std::pair<bool, vk::UniqueCommandBuffer>> command_buffers{};

    std::unique_ptr<DescriptorPool> descriptor_pool;
    std::unique_ptr<DescriptorSetLayout> descriptor_set_layout;
    vk::UniquePipelineLayout graphics_pipeline_layout;
    std::unique_ptr<GraphicsPipeline> graphics_pipeline;

    std::vector<std::unique_ptr<Buffer>> global_ubo;
    std::unique_ptr<Buffer> staged_image;
    std::vector<std::unique_ptr<Image>> image_buffer;
    vk::UniqueSampler image_sampler;
    std::vector<vk::UniqueDescriptorSet> descriptor_sets;

   public:
    App(std::uint32_t width, std::uint32_t height);
    ~App();

    // assume data size = width * height * 4 bytes
    void render(void* data);
    bool should_close() { return window.should_close(); }

    App(const App&) = delete;
    App& operator=(const App&) = delete;
    App(App&&) = delete;
    App& operator=(App&&) = delete;

   private:
    void create_swap_chain();
    void update_global_ubo(std::size_t i);
    void record_command_buffer(std::uint32_t i);
    std::optional<std::uint32_t> acquire_next_frame();
    void submit_frame(std::uint32_t i);
};

}  // namespace aglea
