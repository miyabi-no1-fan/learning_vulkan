#include "model.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

Model::Model(Device& device, const std::vector<Vertex>& vertices) : device(device) {
    create_vertex_buffers(vertices);
}

Model::~Model() {
    vkDestroyBuffer(device.device(), vertex_buffer, nullptr);
    vkFreeMemory(device.device(), vertex_buffer_memory, nullptr);
}

void Model::create_vertex_buffers(const std::vector<Vertex>& vertices) {
    vertex_count = static_cast<uint32_t>(vertices.size());
    assert(vertex_count >= 3 && "Vertex count must be at least 3");

    VkDeviceSize buffer_size = vertex_count * sizeof(vertices[0]);

    device.createBuffer(
        buffer_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertex_buffer,
        vertex_buffer_memory  //
    );

    void* data{};
    {
        auto res = vkMapMemory(device.device(), vertex_buffer_memory, 0, buffer_size, 0, &data);
        if (res != VK_SUCCESS) {
            throw std::runtime_error("Can't map memory. Vulkan Error code: " + std::to_string(res));
        }
    }
    std::memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
    vkUnmapMemory(device.device(), vertex_buffer_memory);
}

void Model::draw(VkCommandBuffer command_buffer) {
    vkCmdDraw(command_buffer, vertex_count, 1, 0, 0);
}

void Model::bind(VkCommandBuffer command_buffer) {
    VkBuffer buffers[] = { vertex_buffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);
}

std::vector<VkVertexInputBindingDescription> Model::Vertex::get_binding_descriptions() {
    std::vector<VkVertexInputBindingDescription> binding_descriptions(1);
    binding_descriptions[0] = {
        .binding = 0,
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
    return binding_descriptions;
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::get_attribute_descriptions() {
    std::vector<VkVertexInputAttributeDescription> attribute_descriptions(1);
    attribute_descriptions[0] = {
        .location = 0,
        .binding = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = 0,
    };
    return attribute_descriptions;
}
