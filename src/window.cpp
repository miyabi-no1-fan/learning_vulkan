#include "window.hpp"

#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <thread>

Window::Window(uint32_t width, uint32_t height, uint32_t fps, std::string name)
    : width(width),
      height(height),
      frame_time(std::chrono::duration<double>(1.0 / static_cast<double>(fps))),
      name(name) {
    initWindow();
}

Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::initWindow() {
    if (glfwInit() != GL_TRUE) {
        throw std::runtime_error("glfw init failed");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        throw std::runtime_error("window creation failed");
    }

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, frame_buffer_resize_callback);
}

void Window::create_window_surface(VkInstance instance, VkSurfaceKHR* surface) {
    VkResult res = glfwCreateWindowSurface(instance, window, nullptr, surface);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface. Vulkan Error Code: " + std::to_string(res));
    }
}

void Window::poll_events() {
    auto end = std::chrono::steady_clock::now();
    auto elapsed = end - start.value_or(end);
    std::this_thread::sleep_for(frame_time - elapsed);
    start = std::chrono::steady_clock::now();
    return glfwPollEvents();
}

void Window::frame_buffer_resize_callback(GLFWwindow* window_, int width, int height) {
    Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window_));
    window->frame_buffer_resized = true;
    window->width = static_cast<uint32_t>(width);
    window->height = static_cast<uint32_t>(height);
}
