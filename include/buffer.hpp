#pragma once
#include <cstdint>
#include <vulkan/vulkan.hpp>

#include "context.hpp"

class Buffer {
    const VulkanContext* ctx = nullptr;

   public:
    vk::UniqueBuffer buffer;
    vk::UniqueDeviceMemory memory;
    void* mapped_memory = nullptr;
    vk::DeviceSize size;

    Buffer(const VulkanContext& ctx) : ctx(&ctx) {}
    ~Buffer() { unmap(); }

    Buffer(Buffer&&) noexcept = default;
    Buffer& operator=(Buffer&&) noexcept = default;

    void allocate(
        vk::DeviceSize size,
        vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eStorageBuffer,
        vk::MemoryPropertyFlags properties = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent  //
    );
    void* map();
    void unmap();

   private:
    std::uint32_t find_memory_type(std::uint32_t typeFilter, vk::MemoryPropertyFlags properties);
};

// F(void*)
template <typename F>
static Buffer staging_buffer(
    const VulkanContext& ctx,
    vk::DeviceSize size,
    vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags properties,
    F&& f) {
    Buffer staged(ctx);
    staged.allocate(
        size,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
    f(staged.map());
    staged.unmap();
    Buffer buffer(ctx);
    buffer.allocate(
        staged.size,
        usage | vk::BufferUsageFlagBits::eTransferDst,
        properties);
    ctx.copy_buffer(buffer.buffer, staged.buffer, staged.size);
    return buffer;
}
