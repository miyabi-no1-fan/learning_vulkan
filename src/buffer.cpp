#include "buffer.hpp"

#include <vulkan/vulkan.hpp>

std::uint32_t Buffer::find_memory_type(std::uint32_t typeFilter, vk::MemoryPropertyFlags properties) {
    vk::PhysicalDeviceMemoryProperties memProperties = ctx->physical_device->getMemoryProperties();
    for (std::uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    throw std::runtime_error("failed to find suitable memory type!");
}

void Buffer::allocate(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) {
    this->size = size;

    buffer = ctx->device->createBufferUnique(vk::BufferCreateInfo(
        {},
        size,
        usage,
        vk::SharingMode::eExclusive,
        1,
        &ctx->family_index));

    vk::MemoryRequirements memRequirements = ctx->device->getBufferMemoryRequirements(*buffer);

    memory =
        ctx->device->allocateMemoryUnique(
            vk::MemoryAllocateInfo(
                memRequirements.size,
                find_memory_type(memRequirements.memoryTypeBits, properties)));

    ctx->device->bindBufferMemory(*buffer, *memory, 0);
}

void* Buffer::map() {
    if (!mapped_memory)
        mapped_memory = static_cast<void*>(ctx->device->mapMemory(*memory, 0, vk::WholeSize));
    return mapped_memory;
}

void Buffer::unmap() {
    if (mapped_memory) {
        ctx->device->unmapMemory(*memory);
        mapped_memory = nullptr;
    }
}
