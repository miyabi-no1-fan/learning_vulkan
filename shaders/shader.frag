#version 450

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec3 color;
layout(location = 1) in vec3 position;
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
    vec3 light_direction = vec3(light_position) - position;
    float attenuation = 1.0 / dot(light_direction, light_direction);  // 1.0 / distance_to_light^2
    float diffuse_lighting = max(dot(normalize(normal), normalize(light_direction)), 0.0);

    vec3 ambient = ambient_light_color.xyz * ambient_light_color.w;
    vec3 light = light_color.xyz * light_color.w * diffuse_lighting * attenuation;

    out_color = vec4((ambient + light) * color, 1.0);
}
