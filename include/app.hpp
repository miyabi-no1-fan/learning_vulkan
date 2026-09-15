#pragma once
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
    SwapChain swap_chain{ device, window.get_extent() };
    std::unique_ptr<Pipeline> pipeline{};
    VkPipelineLayout pipeline_layout{};
    std::vector<VkCommandBuffer> command_buffers{};
    std::unique_ptr<Model> model{};

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
    void draw_frame();

    void load_models();
};
