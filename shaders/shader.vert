#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec2 frag_position;
layout(location = 1) out vec3 frag_color;

layout(push_constant) uniform Push {
    mat2x2 transform;
    vec2 shift;
}
push;

void main() {
    gl_Position = vec4(push.transform * position + push.shift, 0.0, 1.0);
    frag_color = color;
    frag_position = position;
}
