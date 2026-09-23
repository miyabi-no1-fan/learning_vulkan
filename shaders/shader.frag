#version 450

layout(location = 0) out vec4 out_color;

layout(location = 1) in vec3 frag_color;

layout(push_constant) uniform Push {
    mat4 model_matrix;
    mat4 normal_matrix;
};

void main() {
    out_color = vec4(frag_color, 1.0);
}
