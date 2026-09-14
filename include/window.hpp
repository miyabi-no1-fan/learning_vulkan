#pragma once
#include <cstdint>
#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Window {
    GLFWwindow* window;

    uint32_t width;
    uint32_t height;
    std::string name;

   public:
    Window(uint32_t width, uint32_t height, std::string name);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool should_close() { return glfwWindowShouldClose(window); }
    void poll_events() { return glfwPollEvents(); }

    VkExtent2D get_extent() { return VkExtent2D{ width, height }; }

    void create_window_surface(VkInstance instance, VkSurfaceKHR* surface);

   private:
    void initWindow();
};
