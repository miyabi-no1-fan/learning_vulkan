#pragma once
#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Window {
    GLFWwindow* window;

    uint32_t width;
    uint32_t height;
    std::chrono::duration<double> frame_time;
    std::optional<std::chrono::time_point<std::chrono::steady_clock>> start{};

    std::string name;

   public:
    Window(uint32_t width, uint32_t height, uint32_t fps, std::string name);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool should_close() { return glfwWindowShouldClose(window); }
    void poll_events();

    VkExtent2D get_extent() { return VkExtent2D{ width, height }; }

    void create_window_surface(VkInstance instance, VkSurfaceKHR* surface);

   private:
    void initWindow();
};
