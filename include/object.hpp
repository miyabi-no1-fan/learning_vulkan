#pragma once
#include <cstddef>
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

#include "buffer.hpp"
#include "context.hpp"
#include "matrix.hpp"

namespace aglea {

struct Vertex {
    vec2 position;
    uvec2 color;

    static std::vector<vk::VertexInputBindingDescription> get_binding_descriptions() {
        return {
            // binding, stride, input rate
            { 0, sizeof(Vertex), vk::VertexInputRate::eVertex },
        };
    }

    static std::vector<vk::VertexInputAttributeDescription> get_attribute_descriptions() {
        return {
            // location, binding, format, offset
            { 0, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, position) },
            { 1, 0, vk::Format::eR32G32Uint, offsetof(Vertex, color) },
        };
    }
};

class Object {
    std::unique_ptr<Buffer> vertex_buffer = nullptr;
    std::unique_ptr<Buffer> index_buffer = nullptr;

   public:
    Object(const Context& ctx, const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices);

    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;
    Object(Object&&) = delete;
    Object& operator=(Object&&) = delete;

    void bind(const vk::UniqueCommandBuffer& command_buffer);
    void draw(const vk::UniqueCommandBuffer& command_buffer);
};

}  // namespace aglea