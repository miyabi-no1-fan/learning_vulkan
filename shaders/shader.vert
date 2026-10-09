#version 450

layout(location = 0) out vec2 frag_color;

layout(set = 0, binding = 0) uniform UBO {
    float scale_width;
    float scale_height;
    int width;
    int height;
};

vec2 positions[4] = {
    vec2(-1, -1),
    vec2(1, -1),
    vec2(-1, 1),
    vec2(1, 1),
};

vec2 colors[4] = {
    vec2(0, 0),
    vec2(1, 0),
    vec2(0, 1),
    vec2(1, 1),
};

void main() {
    vec2 position = positions[gl_VertexIndex];
    position.x *= scale_width;
    position.y *= scale_height;
    gl_Position = vec4(position, 0, 1);
    frag_color = colors[gl_VertexIndex];
}
