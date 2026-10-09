#pragma once
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "context.hpp"
#include "descriptors.hpp"
#include "pipeline.hpp"
#include "swap_chain.hpp"
#include "window.hpp"

namespace aglea {

struct GlobalUBO {
    float scale_width;
    float scale_height;
    int width;
    int height;
};

class App {
    //
   public:
    static constexpr std::uint32_t DEFAULT_WIDTH = 1920;
    static constexpr std::uint32_t DEFAULT_HEIGHT = 1080;
    static constexpr const char* NAME = "Aglea";

   private:
    Window window{ DEFAULT_WIDTH, DEFAULT_HEIGHT, NAME, 1.0 / 60.0 };
    Context ctx{ window };

    std::unique_ptr<SwapChain> swap_chain{};
    std::vector<std::pair<bool, vk::UniqueCommandBuffer>> command_buffers{};

    std::unique_ptr<DescriptorPool> descriptor_pool;
    std::unique_ptr<DescriptorSetLayout> descriptor_set_layout;
    vk::UniquePipelineLayout graphics_pipeline_layout;
    std::unique_ptr<GraphicsPipeline> graphics_pipeline;

   public:
    App();
    void run();

    App(const App&) = delete;
    App& operator=(const App&) = delete;
    App(App&&) = delete;
    App& operator=(App&&) = delete;

   private:
    void create_swap_chain();
    void record_command_buffer(std::uint32_t i, const vk::UniqueDescriptorSet& descriptor_set);
    std::optional<std::uint32_t> acquire_next_frame();
    void submit_frame(std::uint32_t i);
};

}  // namespace aglea
