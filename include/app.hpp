#pragma once
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "descriptors.hpp"
#include "device.hpp"
#include "object.hpp"
#include "renderer.hpp"
#include "window.hpp"

struct GlobalUniformBuffer {
    alignas(16) glm::mat4x4 projection_view{ 1.f };
    alignas(16) glm::vec3 light_direction = glm::normalize(glm::vec3(1.f, -3.f, -1.f));
};

class App {
   private:
    Window window{ WIDTH, HEIGHT, 0.f, NAME };
    Device device{ window };
    Renderer renderer{ window, device };

    std::unique_ptr<DescriptorPool> global_descriptor_pool{};
    std::vector<Object> objects{};

   public:
    static constexpr uint32_t WIDTH = 1920;
    static constexpr uint32_t HEIGHT = 1080;
    static constexpr uint32_t MAX_FPS = 60;
    static constexpr double MAX_FRAME_TIME = 1.0 / MAX_FPS;
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
