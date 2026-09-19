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

    struct Transform {
        // search for intrinsics rotation and extrinsics rotation
        alignas(16) glm::vec3 rotation{};
        alignas(16) glm::vec3 translation{};
        alignas(16) glm::vec3 scale{ 1.f, 1.f, 1.f };
        alignas(16) glm::mat4x4 projection_view{ 1.f };
    } transform{};
    static_assert(sizeof(Transform) <= 128, "The Vulkan spec only guaranteed 128 bytes of push constant");

    std::function<void(Object& object, float dt, const glm::mat4x4& projection_view)> render = [](Object& object, float _, const glm::mat4x4& projection_view) -> void {
        object.transform.projection_view = projection_view;
    };

   private:
    id_t id;
};
