#version 450
#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) in vec2 color;

layout(set = 0, binding = 1) uniform sampler2D image;

layout(location = 0) out vec4 out_color;

void main() {
    out_color = texture(image, color);
}
