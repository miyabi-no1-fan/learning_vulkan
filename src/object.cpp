#include "object.hpp"

// Matrix corrsponds to Offset * Ry * Rx * Rz * Scalar
// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
glm::mat4x4 Object::transform_matrix() {
    const float c3 = cos(rotation.z);
    const float s3 = sin(rotation.z);
    const float c2 = cos(rotation.x);
    const float s2 = sin(rotation.x);
    const float c1 = cos(rotation.y);
    const float s1 = sin(rotation.y);
    return glm::mat4x4{
        {
            scale.x * (c1 * c3 + s1 * s2 * s3),
            scale.x * (c2 * s3),
            scale.x * (c1 * s2 * s3 - c3 * s1),
            0.f,
        },
        {
            scale.y * (c3 * s1 * s2 - c1 * s3),
            scale.y * (c2 * c3),
            scale.y * (c1 * c3 * s2 + s1 * s3),
            0.f,
        },
        {
            scale.z * (c2 * s1),
            scale.z * (-s2),
            scale.z * (c1 * c2),
            0.f,
        },
        { translation.x, translation.y, translation.z, 1.f }
    };
}

glm::mat4x4 Object::normal_matrix() {
    const float c3 = cos(rotation.z);
    const float s3 = sin(rotation.z);
    const float c2 = cos(rotation.x);
    const float s2 = sin(rotation.x);
    const float c1 = cos(rotation.y);
    const float s1 = sin(rotation.y);
    const glm::vec3 invScale = 1.f / scale;
    return glm::mat4x4{
        {
            invScale.x * (c1 * c3 + s1 * s2 * s3),
            invScale.x * (c2 * s3),
            invScale.x * (c1 * s2 * s3 - c3 * s1),
            0.f,
        },
        {
            invScale.y * (c3 * s1 * s2 - c1 * s3),
            invScale.y * (c2 * c3),
            invScale.y * (c1 * c3 * s2 + s1 * s3),
            0.f,
        },
        {
            invScale.z * (c2 * s1),
            invScale.z * (-s2),
            invScale.z * (c1 * c2),
            0.f,
        },
        { 0.f, 0.f, 0.f, 1.f },
    };
}
