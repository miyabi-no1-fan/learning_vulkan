#version 450
#extension GL_EXT_samplerless_texture_functions : require

layout(location = 0) in vec2 color;

layout(set = 0, binding = 1) uniform texture2D image;

layout(location = 0) out vec4 out_color;

void main() {
    ivec2 extent = textureSize(image, 0);
    ivec2 coord = ivec2(color * vec2(extent));
    coord = clamp(coord, ivec2(0, 0), extent - ivec2(1, 1));
    out_color = texelFetch(image, coord, 0);
}
