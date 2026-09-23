#include "model.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#pragma GCC diagnostic pop

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "utils.hpp"

namespace std {
template <>
struct hash<Model::Vertex> {
    size_t operator()(Model::Vertex const& vertex) const {
        size_t seed = 0;
        hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.texcoord);
        return seed;
    }
};
}  // namespace std

Model::Model(Device& device, const Builder& builder) : device(device) {
    create_vertex_buffers(builder.vertices);
    create_index_buffers(builder.indices);
}

Model::~Model() {}

std::unique_ptr<Model> Model::create_model_from_file(Device& device, const std::string& path) {
    Builder builder{};
    builder.load_models(path);
    return std::make_unique<Model>(device, builder);
}

void Model::create_vertex_buffers(const std::vector<Vertex>& vertices) {
    assert(vertices.size() >= 3 && "Vertex count must be at least 3");

    Buffer staging_buffer(
        device,
        sizeof(vertices[0]),
        vertices.size(),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        1  //
    );

    staging_buffer.map();
    staging_buffer.write_to_buffer((void*)vertices.data());
    staging_buffer.unmap();

    vertex_buffer = std::make_unique<Buffer>(
        device,
        sizeof(vertices[0]),
        vertices.size(),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        1  //
    );

    device.copyBuffer(staging_buffer.get_buffer(), vertex_buffer->get_buffer(), vertex_buffer->get_buffer_size());
}

void Model::create_index_buffers(const std::vector<uint32_t>& indices) {
    if (indices.empty()) {
        index_buffer = nullptr;
        return;
    }

    Buffer staging_buffer(
        device,
        sizeof(indices[0]),
        indices.size(),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        1  //
    );

    staging_buffer.map();
    staging_buffer.write_to_buffer((void*)indices.data());
    staging_buffer.unmap();

    index_buffer = std::make_unique<Buffer>(
        device,
        sizeof(indices[0]),
        indices.size(),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        1  //
    );

    device.copyBuffer(staging_buffer.get_buffer(), index_buffer->get_buffer(), index_buffer->get_buffer_size());
}

void Model::draw(VkCommandBuffer command_buffer) {
    if (index_buffer) {
        vkCmdDrawIndexed(command_buffer, index_buffer->get_instance_count(), 1, 0, 0, 0);
    } else {
        vkCmdDraw(command_buffer, vertex_buffer->get_instance_count(), 1, 0, 0);
    }
}

void Model::bind(VkCommandBuffer command_buffer) {
    VkBuffer buffers[] = { vertex_buffer->get_buffer() };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(command_buffer, 0, 1, buffers, offsets);
    if (index_buffer) {
        vkCmdBindIndexBuffer(command_buffer, index_buffer->get_buffer(), 0, VK_INDEX_TYPE_UINT32);
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
        { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) },
        { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) },
        { 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) },
        { 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texcoord) },
    };
}

void Model::Builder::load_models(const std::string& path) {
    tinyobj::attrib_t attrib{};
    std::vector<tinyobj::shape_t> shapes{};
    std::vector<tinyobj::material_t> materials{};
    std::string warn{}, err{};

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
        throw std::runtime_error(warn + err);
    }

    vertices.clear();
    indices.clear();

    std::unordered_map<Vertex, uint32_t> unique_vertices{};
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            if (index.vertex_index >= 0) {
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };

                vertex.color = {
                    attrib.colors[3 * index.vertex_index + 0],
                    attrib.colors[3 * index.vertex_index + 1],
                    attrib.colors[3 * index.vertex_index + 2],
                };
            }

            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };
            }

            if (index.texcoord_index >= 0) {
                vertex.texcoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
                };
            }

            if (unique_vertices.count(vertex) == 0) {
                unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(unique_vertices[vertex]);
        }
    }
}
