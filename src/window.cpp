#include "window.hpp"

#include <cmath>
#include <stdexcept>
#include <thread>
#include <vulkan/vulkan.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace aglea {

Window::Window(std::uint32_t width, std::uint32_t height, const std::string& name, double frame_time)
    : width(width), height(height), name(name), frame_time(frame_time) {
    if (glfwInit() != GLFW_TRUE)
        throw std::runtime_error("Failed to initialize window.");
    if (glfwVulkanSupported() != GLFW_TRUE)
        throw std::runtime_error("GLFW does not support Vulkan.");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    if (window == nullptr)
        throw std::runtime_error("Failed to create window.");

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, frame_buffer_resize_callback);
}

Window::~Window() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::poll_events() {
    if (prev_time) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration<double>(now - *prev_time);
        std::chrono::duration<double> delta(std::fmod(elapsed.count(), frame_time.count()));
        std::this_thread::sleep_for(frame_time - delta);
        prev_time = std::chrono::steady_clock::now();
    } else {
        std::this_thread::sleep_for(frame_time);
    }
    return glfwPollEvents();
}

vk::UniqueSurfaceKHR Window::create_window_surface(const vk::UniqueInstance& instance) const {
    VkSurfaceKHR surface;
    VkResult res = glfwCreateWindowSurface(*instance, window, nullptr, &surface);
    if (res != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface.");
    return vk::UniqueSurfaceKHR(surface, *instance);
}

void Window::frame_buffer_resize_callback(GLFWwindow* window_, int width, int height) {
    Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window_));
    window->frame_buffer_resized = true;
    window->width = static_cast<uint32_t>(width);
    window->height = static_cast<uint32_t>(height);
}

}  // namespace aglea
