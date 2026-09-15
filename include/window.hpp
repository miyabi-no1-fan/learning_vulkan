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

    bool frame_buffer_resized = false;

    std::string name;

   public:
    Window(uint32_t width, uint32_t height, uint32_t fps, std::string name);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool should_close() { return glfwWindowShouldClose(window); }
    void poll_events();

    VkExtent2D get_extent() { return VkExtent2D{ width, height }; }

    bool was_window_resized() { return frame_buffer_resized; }
    void reset_window_resized_flag() { frame_buffer_resized = false; }

    void create_window_surface(VkInstance instance, VkSurfaceKHR* surface);

   private:
    void initWindow();
    static void frame_buffer_resize_callback(GLFWwindow* window_, int width, int height);
};
