#pragma once
#include <cstdint>
#include <memory>
#include <string>
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

    bool has_index_buffer = false;
    VkBuffer index_buffer{};
    VkDeviceMemory index_buffer_memory{};
    uint32_t index_count{};

   public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 normal{};  // the vector n of a plane
        glm::vec2 texcoord{};

        // Each corner (vertex) of a triangle gets two sets of numbers:
        // 1. Where it is in 3D space (position)
        // 2. Which spot on the photo it should show (texture coordinate)

        static std::vector<VkVertexInputBindingDescription> get_binding_descriptions();
        static std::vector<VkVertexInputAttributeDescription> get_attribute_descriptions();

        bool operator==(const Vertex& other) const {
            return position == other.position &&
                   color == other.color &&
                   normal == other.normal &&
                   texcoord == other.texcoord;
        }
    };

    struct Builder {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};

        void load_models(const std::string& path);
    };

    Model(Device& device, const Builder& builder);
    ~Model();

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    static std::unique_ptr<Model> create_model_from_file(Device& device, const std::string& path);

    void bind(VkCommandBuffer command_buffer);
    void draw(VkCommandBuffer command_buffer);

   private:
    void create_vertex_buffers(const std::vector<Vertex>& vertices);
    void create_index_buffers(const std::vector<uint32_t>& indices);
};
