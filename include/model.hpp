#pragma once
#include <cstdint>
#include <vector>

#include "device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class Model {
   private:
    Device& device;
    VkBuffer vertex_buffer{};
    VkDeviceMemory vertex_buffer_memory{};
    uint32_t vertex_count{};

   public:
    struct Vertex {
        glm::vec2 position;

        static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
        static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();
    };

    Model(Device& device, const std::vector<Vertex>& vertices);
    ~Model();

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    void bind(VkCommandBuffer command_buffer);
    void draw(VkCommandBuffer command_buffer);

   private:
    void create_vertex_buffers(const std::vector<Vertex>& vertices);
};
