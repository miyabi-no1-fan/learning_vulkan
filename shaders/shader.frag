#version 450

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

void main() {
    float radius = 0.5f;
    float dist = length(position);
    if (dist > radius) {
        discard;
    }
    out_color = vec4(color, 1.0);
}
