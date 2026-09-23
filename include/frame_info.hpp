#pragma once
#include <cstdint>

#include "object.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct FrameInfo {
    uint32_t index;
    double time;
    VkCommandBuffer command_buffer;
    VkDescriptorSet global_descriptor_set;
    Object::Map& objects;
};
