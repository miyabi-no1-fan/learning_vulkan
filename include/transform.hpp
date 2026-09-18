#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

struct Transform {
    glm::mat4x4 mat{ 1.0f };

    enum Planes {
        Oxy,
        Oxz,
        Oyz,
    };

    void offset(float dx, float dy, float dz);
    void rotate(float rad, Planes p);
    void scale(glm::vec3 scalar);
};
