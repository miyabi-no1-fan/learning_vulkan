#pragma once
#include <array>
#include <memory>
#include <vector>

#include "object.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class Triangle : public Object {
   public:
    ~Triangle() override {}

    Triangle(Device& device, const std::array<Model::Vertex, 3>& vertices) {
        model = std::make_shared<Model>(device, std::vector(vertices.cbegin(), vertices.cend()));
    }

    void render() override {
    }
};
