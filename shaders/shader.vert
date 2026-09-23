#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 texcoord;

layout(location = 1) out vec3 frag_color;

layout(push_constant) uniform Push {
    mat4 transform_matrix;
    mat4 normal_matrix;
};

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -3.0, -1.0));
const float AMBIENT = 0.02;

void main() {
    gl_Position = transform_matrix * vec4(position, 1.0);
    vec3 normal = normalize(mat3(normal_matrix) * normal);
    float light_intensity = AMBIENT + max(dot(normal, DIRECTION_TO_LIGHT), 0.0);
    frag_color = light_intensity * color;
}
