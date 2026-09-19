#pragma once

#include "object.hpp"
#include "window.hpp"

class Controller {
    double prev_cursor_xpos;
    double prev_cursor_ypos;

   public:
    struct Move {
        int left = GLFW_KEY_A;
        int right = GLFW_KEY_D;
        int forward = GLFW_KEY_W;
        int backward = GLFW_KEY_S;
        int up = GLFW_KEY_SPACE;
        int down = GLFW_KEY_LEFT_CONTROL;
    } keys{};

    void move(const Window& window, float dt, Object& object);

    float move_speed = 3.f;
    float cursor_speed = 1.0f;
};
