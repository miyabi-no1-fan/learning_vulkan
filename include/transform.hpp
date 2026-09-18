#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

struct Transform {
    glm::vec3 offset{};
    glm::vec3 rotate{};
    glm::vec3 scalar{ 1.f, 1.f, 1.f };
    glm::mat4 mat();
};
