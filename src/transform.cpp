#include "transform.hpp"

enum Planes {
    Oxy,
    Oxz,
    Oyz,
};

glm::mat4 rotate_mat(float rad, Planes p);

glm::mat4 Transform::mat() {
    glm::mat4 transform{ 1.f };

    transform =
        rotate_mat(rotate.z, Planes::Oxy) *
        rotate_mat(rotate.y, Planes::Oxz) *
        rotate_mat(rotate.x, Planes::Oyz);

    transform = {
        scalar.x * transform[0],
        scalar.y * transform[1],
        scalar.z * transform[2],
        transform[3],
    };

    transform[3][0] += offset.x;
    transform[3][1] += offset.y;
    transform[3][2] += offset.z;

    return transform;
}

glm::mat4 rotate_mat(float rad, Planes p) {
    // stupid glm use column-major
    switch (p) {
        case Planes::Oxy:
            return glm::mat4x4{
                { cos(rad), sin(rad), 0.f, 0.f },
                { -sin(rad), cos(rad), 0.f, 0.f },
                { 0.f, 0.f, 1.f, 0.f },
                { 0.f, 0.f, 0.f, 1.f },
            };
            // return glm::transpose(glm::mat4x4{
            //     { cos(rad), -sin(rad), 0.f, 0.f },
            //     { sin(rad), cos(rad), 0.f, 0.f },
            //     { 0.f,      0.f,      1.f, 0.f },
            //     { 0.f,      0.f,      0.f, 1.f },
            // });
        case Planes::Oxz:
            return glm::mat4x4{
                { cos(rad), 0.f, sin(rad), 0.f },
                { 0.f, 1.f, 0.f, 0.f },
                { -sin(rad), 0.f, cos(rad), 0.f },
                { 0.f, 0.f, 0.f, 1.f },
            };
            // return glm::transpose(glm::mat4x4{
            //     { cos(rad), 0.f, -sin(rad), 0.f },
            //     { 0.f,      1.f, 0.f,      0.f },
            //     { sin(rad), 0.f, cos(rad), 0.f },
            //     { 0.f,      0.f, 0.f,      1.f },
            // });
        case Planes::Oyz:
            return glm::mat4x4{
                { 1.f, 0.f, 0.f, 0.f },
                { 0.f, cos(rad), sin(rad), 0.f },
                { 0.f, -sin(rad), cos(rad), 0.f },
                { 0.f, 0.f, 0.f, 1.f },
            };
            // return glm::transpose(glm::mat4x4{
            //     { 1.f, 0.f,      0.f,       0.f },
            //     { 0.f, cos(rad), -sin(rad), 0.f },
            //     { 0.f, sin(rad), cos(rad), 0.f },
            //     { 0.f, 0.f,      0.f,      1.f },
            // });
    }
}
