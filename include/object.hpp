#pragma once
#include <cstdint>
#include <functional>
#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "model.hpp"

class Object {
   public:
    using id_t = uint64_t;

    Object() {
        static uint64_t id = 0;
        this->id = ++id;
    }

    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;
    Object(Object&&) = default;
    Object& operator=(Object&&) = default;

    id_t get_id() const { return this->id; }

    std::shared_ptr<Model> model{};

    // search for intrinsics rotation and extrinsics rotation
    glm::vec3 rotation{};
    glm::vec3 translation{};
    glm::vec3 scale{ 1.f, 1.f, 1.f };
    glm::mat4x4 projection_view{ 1.f };

    glm::mat4x4 transform_matrix();
    glm::mat4x4 normal_matrix();

    std::function<void(Object& object, float dt, const glm::mat4x4& projection_view)> render = [](Object& object, float _, const glm::mat4x4& projection_view) -> void {
        object.projection_view = projection_view;
    };

   private:
    id_t id;
};
