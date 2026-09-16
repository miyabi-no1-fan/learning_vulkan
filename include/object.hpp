#pragma once
#include <cstdint>
#include <memory>

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

    virtual ~Object() = default;

    id_t get_id() const { return this->id; }

    std::shared_ptr<Model> model{};

    virtual void render() = 0;

   private:
    id_t id;
};
