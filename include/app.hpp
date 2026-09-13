#pragma once
#include <cstdint>
#include <filesystem>

#include "device.hpp"
#include "pipeline.hpp"
#include "window.hpp"

class App {
   public:
    static constexpr uint32_t WIDTH = 1920;
    static constexpr uint32_t HEIGHT = 1080;
    static constexpr const char* NAME = "test-app";

    static constexpr const char* VERTEX_SHADER_SRC = "build/shaders/shader.vert.spv";
    static constexpr const char* FRAGMENT_SHADER_SRC = "build/shaders/shader.vert.spv";

    App(const App&) = delete;
    void operator=(const App&) = delete;

    App() {
        if (!std::filesystem::exists(VERTEX_SHADER_SRC)) {
            throw std::runtime_error("Can't find shader file: " + std::string(VERTEX_SHADER_SRC));
        }
        if (!std::filesystem::exists(FRAGMENT_SHADER_SRC)) {
            throw std::runtime_error("Can't find shader file: " + std::string(FRAGMENT_SHADER_SRC));
        }
    }

    void run();

   private:
    Window window{ WIDTH, HEIGHT, NAME };
    Device device{ window };
    Pipeline pipeline{
        device,
        VERTEX_SHADER_SRC,
        FRAGMENT_SHADER_SRC,
        Pipeline::default_config_info(WIDTH, HEIGHT)  //
    };
};
