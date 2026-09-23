#version 450

layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec3 frag_position;
layout(location = 2) out vec3 frag_normal;
layout(location = 3) out vec3 frag_texcoord;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 texcoord;

layout(set = 0, binding = 0) uniform GlobalUniformBuffer {
    mat4 projection_view;
    vec4 light_position;
    vec4 light_color;
    vec4 ambient_light_color;
};

layout(push_constant) uniform Push {
    mat4 model_matrix;
    mat4 normal_matrix;
};

void main() {
    vec4 position_in_world_space = model_matrix * vec4(position, 1.0);
    gl_Position = projection_view * position_in_world_space;

    frag_color = color;
    frag_position = vec3(position_in_world_space);
    frag_normal = mat3(normal_matrix) * normal;
    frag_texcoord = texcoord;
}
