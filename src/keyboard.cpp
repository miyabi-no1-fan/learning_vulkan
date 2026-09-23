#include "keyboard.hpp"

#include <cmath>
#include <limits>
#include <numbers>

void Controller::move(const Window& window, float dt, Object& object) {
    double xpos{}, ypos{};
    glfwGetCursorPos(window.get_GLFWwindow(), &xpos, &ypos);

    float dx{}, dy{};
    dx = xpos - prev_cursor_xpos;
    dy = ypos - prev_cursor_ypos;
    prev_cursor_xpos = xpos;
    prev_cursor_ypos = ypos;

    object.rotation.y += cursor_speed * dt * dx;
    object.rotation.x -= cursor_speed * dt * dy;

    object.rotation = {
        std::fmodf(object.rotation.x, std::numbers::pi * 2.f),
        std::fmodf(object.rotation.y, std::numbers::pi * 2.f),
        std::fmodf(object.rotation.z, std::numbers::pi * 2.f),
    };

    float pitch = object.rotation.x;
    float yaw = object.rotation.y;

    const glm::vec3 right = { std::cos(yaw), 0.f, -std::sin(yaw) };
    const glm::vec3 foward = { std::sin(yaw) * std::cos(pitch), -std::sin(pitch), std::cos(yaw) * std::cos(pitch) };
    const glm::vec3 up = { 0.f, -1.f, 0.f };

    glm::vec3 move{ 0.f };
    if (glfwGetKey(window.get_GLFWwindow(), keys.right) == GLFW_PRESS) move += right;
    if (glfwGetKey(window.get_GLFWwindow(), keys.left) == GLFW_PRESS) move -= right;
    if (glfwGetKey(window.get_GLFWwindow(), keys.forward) == GLFW_PRESS) move += foward;
    if (glfwGetKey(window.get_GLFWwindow(), keys.backward) == GLFW_PRESS) move -= foward;
    if (glfwGetKey(window.get_GLFWwindow(), keys.up) == GLFW_PRESS) move += up;
    if (glfwGetKey(window.get_GLFWwindow(), keys.down) == GLFW_PRESS) move -= up;

    if (glm::length(move) > std::numeric_limits<float>::epsilon()) {
        object.translation += move_speed * dt * glm::normalize(move);
    }
}
