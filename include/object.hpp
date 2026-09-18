#pragma once
#include <cstdint>
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

    struct PushConstant {
        alignas(16) glm::mat2x2 matrix{ 1.0f };
        alignas(8) glm::vec2 shift{};
    } transform2d{};

   private:
    id_t id;
};
