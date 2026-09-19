#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class Camera {
    glm::mat4 projection{ 1.f };
    glm::mat4 view{ 1.f };

   public:
    enum View {
        Default,
    };

    void set_orthographic_projection(float left, float right, float top, float bottom, float near, float far);
    void set_perspective_projection(float fovy, float aspect, float near, float far);

    void set_view_direction(glm::vec3 position, glm::vec3 direction, View option = Default);
    void set_view_target(glm::vec3 position, glm::vec3 target, View option = Default);
    void set_view_yxz(glm::vec3 position, glm::vec3 rotate);

    const glm::mat4 get_projection() const { return projection; }
    const glm::mat4 get_view() const { return view; }
};
