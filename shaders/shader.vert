#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 1) out vec3 frag_color;

layout(push_constant) uniform Push {
    vec3 rotation;
    vec3 translation;
    vec3 scale;
    mat4 projection_view;
};

// Matrix corrsponds to Offset * Ry * Rx * Rz * Scalar
// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
mat4 transform() {
    const float c3 = cos(rotation.z);
    const float s3 = sin(rotation.z);
    const float c2 = cos(rotation.x);
    const float s2 = sin(rotation.x);
    const float c1 = cos(rotation.y);
    const float s1 = sin(rotation.y);
    mat4 a = {
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
    return a;
}

void main() {
    gl_Position = projection_view * transform() * vec4(position, 1.0);
    frag_color = color;
}
