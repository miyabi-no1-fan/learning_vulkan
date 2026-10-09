#include "object.hpp"

#include <cassert>
#include <cstring>
#include <memory>
#include <vulkan/vulkan.hpp>

#include "buffer.hpp"

namespace aglea {

Object::Object(const Context& ctx, const std::vector<Vertex>& vertices, const std::vector<std::uint32_t>& indices) {
    assert(vertices.size() >= 3 && "Vertex count must be at least 3");

    vertex_buffer = std::make_unique<Buffer>(
        staging_buffer(
            ctx,
            sizeof(vertices[0]),
            vertices.size(),
            vk::BufferUsageFlagBits::eVertexBuffer,
            vk::MemoryPropertyFlagBits::eDeviceLocal,
            [&vertices](void* data) {
                std::memcpy(data, vertices.data(), vertices.size() * sizeof(vertices[0]));
            }));

    if (!indices.empty()) {
        index_buffer = std::make_unique<Buffer>(
            staging_buffer(
                ctx,
                sizeof(indices[0]),
                indices.size(),
                vk::BufferUsageFlagBits::eIndexBuffer,
                vk::MemoryPropertyFlagBits::eDeviceLocal,
                [&indices](void* data) {
                    std::memcpy(data, indices.data(), indices.size() * sizeof(indices[0]));
                }));
    }
}

void Object::bind(const vk::UniqueCommandBuffer& command_buffer) {
    command_buffer->bindVertexBuffers(0, *vertex_buffer->get_buffer(), { 0 });
    if (index_buffer) {
        command_buffer->bindIndexBuffer(*index_buffer->get_buffer(), 0, vk::IndexType::eUint32);
    }
}

void Object::draw(const vk::UniqueCommandBuffer& command_buffer) {
    if (index_buffer) {
        command_buffer->drawIndexed(index_buffer->get_instance_count(), 1, 0, 0, 0);
    } else {
        command_buffer->draw(vertex_buffer->get_instance_count(), 1, 0, 0);
    }
}

}  // namespace aglea
