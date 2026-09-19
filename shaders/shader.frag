#version 450

layout(location = 0) out vec4 out_color;

layout(location = 1) in vec3 frag_color;

layout(push_constant) uniform Push {
    vec3 rotation;
    vec3 translation;
    vec3 scale;
    mat4 projection_view;
};

void main() {
    out_color = vec4(frag_color, 1.0);
}
