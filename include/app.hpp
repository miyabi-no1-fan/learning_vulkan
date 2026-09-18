#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

#include "device.hpp"
#include "object.hpp"
#include "renderer.hpp"
#include "window.hpp"

class App {
   private:
    Window window{ WIDTH, HEIGHT, FPS, NAME };
    Device device{ window };
    Renderer renderer{ window, device };

    std::vector<Object> objects{};

   public:
    static constexpr uint32_t WIDTH = 1920;
    static constexpr uint32_t HEIGHT = 1080;
    static constexpr uint32_t FPS = 60;
    static constexpr float FRAME_TIME = 1.0f / FPS;
    static constexpr const char* NAME = "test-app";

    App();
    ~App();

    App(const App&) = delete;
    App& operator=(const App&) = delete;

    void run();

   private:
    size_t current_frame = 0;
    void load_objects();
};
