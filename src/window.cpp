#include "window.hpp"

#include <stdexcept>
#include <string>

Window::Window(uint32_t width, uint32_t height, std::string name) : width(width), height(height), name(name) {
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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        throw std::runtime_error("window creation failed");
    }
}

void Window::create_window_surface(VkInstance instance, VkSurfaceKHR* surface) {
    VkResult res = glfwCreateWindowSurface(instance, window, nullptr, surface);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface. Vulkan Error Code: " + std::to_string(res));
    }
}
