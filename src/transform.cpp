#include "transform.hpp"

// Matrix corrsponds to Offset * Ry * Rx * Rz * Scalar
// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
glm::mat4 Transform::mat() {
    const float c3 = glm::cos(rotate.z);
    const float s3 = glm::sin(rotate.z);
    const float c2 = glm::cos(rotate.x);
    const float s2 = glm::sin(rotate.x);
    const float c1 = glm::cos(rotate.y);
    const float s1 = glm::sin(rotate.y);
    return glm::mat4{
        {
            scalar.x * (c1 * c3 + s1 * s2 * s3),
            scalar.x * (c2 * s3),
            scalar.x * (c1 * s2 * s3 - c3 * s1),
            0.0f,
        },
        {
            scalar.y * (c3 * s1 * s2 - c1 * s3),
            scalar.y * (c2 * c3),
            scalar.y * (c1 * c3 * s2 + s1 * s3),
            0.0f,
        },
        {
            scalar.z * (c2 * s1),
            scalar.z * (-s2),
            scalar.z * (c1 * c2),
            0.0f,
        },
        { offset.x, offset.y, offset.z, 1.0f }
    };
}
