#version 450

layout(location = 0) in vec2 color;

layout(set = 0, binding = 0) uniform UBO {
    float scale_width;
    float scale_height;
    int width;
    int height;
};

layout(set = 0, binding = 1) uniform samplerBuffer image;

layout(location = 0) out vec4 out_color;

void main() {
    ivec2 coord = ivec2(round(color * vec2(width, height)));
    coord.x = min(coord.x, width);
    coord.y = min(coord.y, height);
    out_color = texelFetch(image, coord.y * width + coord.x);
}
