#pragma once
#include <cstdint>
#include <memory>

#include "model.hpp"
#include "transform.hpp"

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

    Transform transform{};

   private:
    id_t id;
};
