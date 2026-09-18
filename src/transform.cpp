#include "transform.hpp"

// #include <cmath>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/matrix.hpp>

void Transform::offset(float dx, float dy, float dz) {
    dx = glm::mod(dx, 1.0f);
    dy = glm::mod(dy, 1.0f);
    dz = glm::mod(dz, 1.0f);
    mat = glm::translate(glm::mat4x4{ 1.f }, { dx, dy, dz }) * mat;
    // mat =
    //     glm::transpose(glm::mat4x4{
    //         { 1.f, 0.f, 0.f, dx },
    //         { 0.f, 1.f, 0.f, dy },
    //         { 0.f, 0.f, 1.f, dz },
    //         { 0.f, 0.f, 0.f, 1.f },
    //     }) *
    //     mat;
}

void Transform::rotate(float rad, Planes p) {
    rad = glm::mod(rad, glm::two_pi<float>());
    switch (p) {
        case Planes::Oxy:
            mat = glm::rotate(mat, rad, { 0.0f, 0.0f, 1.0f });
            break;
        case Planes::Oxz:
            mat = glm::rotate(mat, rad, { 0.0f, 1.0f, 0.0f });
            break;
        case Planes::Oyz:
            mat = glm::rotate(mat, rad, { 1.0f, 0.0f, 0.0f });
            break;
    }
    // float sinx = std::sin(rad);
    // float cosx = std::cos(rad);
    // switch (p) {
    //     case Planes::Oxy: {
    //         mat = glm::transpose(glm::mat4x4{
    //             { cosx, -sinx, 0.0f, 0.0f },
    //             { sinx, cosx, 0.0f, 0.0f },
    //             { 0.0f, 0.0f, 1.0f, 0.0f },
    //             { 0.0f, 0.0f, 0.0f, 1.0f },
    //         }) * mat;
    //         break;
    //     }
    //     case Planes::Oxz: {
    //         mat = glm::transpose(glm::mat4x4{
    //             { cosx, 0.0f, -sinx, 0.0f },
    //             { 0.0f, 1.0f, 0.0f, 0.0f },
    //             { sinx, 0.0f, cosx, 0.0f },
    //             { 0.0f, 0.0f, 0.0f, 1.0f },
    //         }) * mat;
    //         break;
    //     }
    //     case Planes::Oyz: {
    //         mat = glm::transpose(glm::mat4x4{
    //             { 1.0f, 0.0f, 0.0f, 0.0f },
    //             { 0.0f, cosx, -sinx, 0.0f },
    //             { 0.0f, sinx, cosx, 0.0f },
    //             { 0.0f, 0.0f, 0.0f, 1.0f },
    //         }) * mat;
    //         break;
    //     }
    // }
}

void Transform::scale(glm::vec3 scalar) {
    float x = glm::mod(scalar.x, 1.0f);
    float y = glm::mod(scalar.y, 1.0f);
    float z = glm::mod(scalar.z, 1.0f);
    mat = glm::scale(mat, { x, y, z });
    // mat = glm::transpose(glm::mat4x4{
    //     { x, 0.0f, 0.0f, 0.0f },
    //     { 0.0f, y, 0.0f, 0.0f },
    //     { 0.0f, 0.0f, z, 0.0f },
    //     { 0.0f, 0.0f, 0.0f, 1.0f },
    // }) * mat;
}
