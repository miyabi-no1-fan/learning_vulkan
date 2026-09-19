#include "window.hpp"

#include <GLFW/glfw3.h>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <thread>

Window::Window(uint32_t width, uint32_t height, double frame_time, std::string name)
    : width(width),
      height(height),
      frame_time(std::chrono::duration<double>(frame_time)),
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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
    if (frame_time.count() > std::numeric_limits<double>::epsilon()) {
        auto end = std::chrono::steady_clock::now();
        auto start = start_time.value_or(end);
        std::chrono::duration<double> elapsed = end - start;

        elapsed = std::chrono::duration<double>(std::fmod(elapsed.count(), frame_time.count()));

        std::this_thread::sleep_for(frame_time - elapsed);

        start_time = std::chrono::steady_clock::now();
    }
    return glfwPollEvents();
}

void Window::frame_buffer_resize_callback(GLFWwindow* window_, int width, int height) {
    Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window_));
    window->frame_buffer_resized = true;
    window->width = static_cast<uint32_t>(width);
    window->height = static_cast<uint32_t>(height);
}
