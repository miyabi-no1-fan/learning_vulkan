#include "model.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

Model::Model(Device& device, const Builder& builder) : device(device) {
    create_vertex_buffers(builder.vertices);
    create_index_buffers(builder.indices);
}

Model::~Model() {
    vkDestroyBuffer(device.device(), vertex_buffer, nullptr);
    vkFreeMemory(device.device(), vertex_buffer_memory, nullptr);
    if (has_index_buffer) {
        vkDestroyBuffer(device.device(), index_buffer, nullptr);
        vkFreeMemory(device.device(), index_buffer_memory, nullptr);
    }
}

void Model::create_vertex_buffers(const std::vector<Vertex>& vertices) {
    vertex_count = static_cast<uint32_t>(vertices.size());
    assert(vertex_count >= 3 && "Vertex count must be at least 3");
    VkDeviceSize buffer_size = vertex_count * sizeof(vertices[0]);

    VkBuffer staging_buffer{};
    VkDeviceMemory staging_buffer_memory{};
    device.createBuffer(
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer,
        staging_buffer_memory  //
    );
    void* data{};
    VkResult res = vkMapMemory(device.device(), staging_buffer_memory, 0, buffer_size, 0, &data);
    if (res != VK_SUCCESS) throw std::runtime_error("Can't map memory. Vulkan Error code: " + std::to_string(res));
    std::memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));
    vkUnmapMemory(device.device(), staging_buffer_memory);

    device.createBuffer(
        buffer_size,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertex_buffer,
        vertex_buffer_memory  //
    );

    device.copyBuffer(staging_buffer, vertex_buffer, buffer_size);

    vkDestroyBuffer(device.device(), staging_buffer, nullptr);
    vkFreeMemory(device.device(), staging_buffer_memory, nullptr);
}

void Model::create_index_buffers(const std::vector<uint32_t>& indices) {
    index_count = static_cast<uint32_t>(indices.size());
    has_index_buffer = index_count > 0;
    if (!has_index_buffer) return;
    VkDeviceSize buffer_size = index_count * sizeof(indices[0]);

    VkBuffer staging_buffer{};
    VkDeviceMemory staging_buffer_memory{};
    device.createBuffer(
        buffer_size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer,
        staging_buffer_memory  //
    );
    void* data{};
    VkResult res = vkMapMemory(device.device(), staging_buffer_memory, 0, buffer_size, 0, &data);
    if (res != VK_SUCCESS) throw std::runtime_error("Can't map memory. Vulkan Error code: " + std::to_string(res));
    std::memcpy(data, indices.data(), static_cast<size_t>(buffer_size));
    vkUnmapMemory(device.device(), staging_buffer_memory);

    device.createBuffer(
        buffer_size,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        index_buffer,
        index_buffer_memory  //
    );

    device.copyBuffer(staging_buffer, index_buffer, buffer_size);

    vkDestroyBuffer(device.device(), staging_buffer, nullptr);
    vkFreeMemory(device.device(), staging_buffer_memory, nullptr);
}

void Model::draw(VkCommandBuffer command_buffer) {
    if (has_index_buffer) {
        vkCmdDrawIndexed(command_buffer, index_count, 1, 0, 0, 0);
    } else {
        vkCmdDraw(command_buffer, vertex_count, 1, 0, 0);
    }
}

void Model::bind(VkCommandBuffer command_buffer) {
    VkBuffer buffers[] = { vertex_buffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);
    if (has_index_buffer) {
        vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
    }
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
    return {
        {
            .location = 0,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, position),
        },
        {
            .location = 1,
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, color),
        }
    };
}
