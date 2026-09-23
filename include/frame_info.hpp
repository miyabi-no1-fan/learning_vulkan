#pragma once
#include <cstdint>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct FrameInfo {
    uint32_t index;
    double time;
    VkCommandBuffer command_buffer;
    VkDescriptorSet global_descriptor_set;
};
