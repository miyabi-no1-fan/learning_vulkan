#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "device.hpp"
#include "model.hpp"
#include "pipeline.hpp"
#include "swap_chain.hpp"
#include "window.hpp"

class App {
   private:
    Window window{ WIDTH, HEIGHT, FPS, NAME };
    Device device{ window };
    std::unique_ptr<SwapChain> swap_chain{};
    std::unique_ptr<Pipeline> pipeline{};
    VkPipelineLayout pipeline_layout{};
    std::vector<VkCommandBuffer> command_buffers{};
    std::vector<std::unique_ptr<Model>> models{};

    struct State {
        std::vector<Model::Vertex> vertices = {};
        size_t current_frame = 0;
    } state{};

   public:
    static constexpr uint32_t WIDTH = 1920;
    static constexpr uint32_t HEIGHT = 1080;
    static constexpr uint32_t FPS = 60;
    static constexpr const char* NAME = "test-app";

    App();
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    void run();

   private:
    static constexpr const char* VERTEX_SHADER_SRC = "shaders/dist/shader.vert.spv";
    static constexpr const char* FRAGMENT_SHADER_SRC = "shaders/dist/shader.frag.spv";

    void create_pipeline_layout();
    void create_pipeline();
    void create_command_buffers();
    void create_swap_chain();

    void free_command_buffers();

    // recreate swap_chain AND pipeline AND command_buffers
    void recreate_swap_chain();

    void record_command_buffer(size_t image_index);
    void draw_frame();
    void load_models();

    void update_model(size_t image_index);
};
