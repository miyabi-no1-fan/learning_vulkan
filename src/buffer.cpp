#include "buffer.hpp"

#include <cassert>
#include <cstring>
#include <stdexcept>
#include <string>

VkDeviceSize Buffer::get_alignment(VkDeviceSize instance_size, VkDeviceSize min_offset_alignment) {
    if (min_offset_alignment > 0) {
        return (instance_size + min_offset_alignment - 1) & ~(min_offset_alignment - 1);
    }
    return instance_size;
}

Buffer::Buffer(
    Device& device,
    VkDeviceSize instance_size,
    uint32_t instance_count,
    VkBufferUsageFlags usage_flags,
    VkMemoryPropertyFlags memory_property_flags,
    VkDeviceSize min_offset_alignment)
    : device{ device },
      instance_count{ instance_count },
      instance_size{ instance_size },
      usage_flags{ usage_flags },
      memory_property_flags{ memory_property_flags } {
    alignment_size = get_alignment(instance_size, min_offset_alignment);
    buffer_size = alignment_size * instance_count;
    device.createBuffer(buffer_size, usage_flags, memory_property_flags, buffer, memory);
}

Buffer::~Buffer() {
    unmap();
    vkDestroyBuffer(device.device(), buffer, nullptr);
    vkFreeMemory(device.device(), memory, nullptr);
}

/**
 * Map a memory range of this buffer. If successful, mapped points to the specified buffer range.
 *
 * @param size (Optional) Size of the memory range to map. Pass VK_WHOLE_SIZE to map the complete
 * buffer range.
 * @param offset (Optional) Byte offset from beginning
 */
void Buffer::map(VkDeviceSize size, VkDeviceSize offset) {
    assert(buffer && memory && "Called map on buffer before create");
    VkResult res = vkMapMemory(device.device(), memory, offset, size, 0, &mapped);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to map buffer memory. Vulkan Error code: " + std::to_string(res));
    }
}

void Buffer::unmap() {
    if (mapped) {
        vkUnmapMemory(device.device(), memory);
        mapped = nullptr;
    }
}

void Buffer::write_to_buffer(void* data, VkDeviceSize size, VkDeviceSize offset) {
    assert(mapped && "Cannot copy to unmapped buffer");

    if (size == VK_WHOLE_SIZE) {
        memcpy(mapped, data, buffer_size);
    } else {
        char* memOffset = (char*)mapped;
        memOffset += offset;
        memcpy(memOffset, data, size);
    }
}

/**
 * Flush a memory range of the buffer to make it visible to the device
 *
 * @note Only required for non-coherent memory
 */
void Buffer::flush(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    VkResult res = vkFlushMappedMemoryRanges(device.device(), 1, &mappedRange);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to flush mapped buffer memory. Vulkan Error code: " + std::to_string(res));
    }
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 */
void Buffer::invalidate(VkDeviceSize size, VkDeviceSize offset) {
    VkMappedMemoryRange mappedRange = {};
    mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mappedRange.memory = memory;
    mappedRange.offset = offset;
    mappedRange.size = size;
    VkResult res = vkInvalidateMappedMemoryRanges(device.device(), 1, &mappedRange);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to invalidate mapped buffer memory. Vulkan Error code: " + std::to_string(res));
    }
}

/**
 * Create a buffer info descriptor
 */
VkDescriptorBufferInfo Buffer::descriptor_info(VkDeviceSize size, VkDeviceSize offset) {
    return VkDescriptorBufferInfo{
        buffer,
        offset,
        size,
    };
}

/**
 * Copies "instanceSize" bytes of data to the mapped buffer at an offset of index * alignmentSize
 */
void Buffer::write_to_index(void* data, int index) {
    write_to_buffer(data, instance_size, index * alignment_size);
}

/**
 *  Flush the memory range at index * alignmentSize of the buffer to make it visible to the device
 */
void Buffer::flush_index(int index) { return flush(alignment_size, index * alignment_size); }

/**
 * Create a buffer info descriptor
 */
VkDescriptorBufferInfo Buffer::index_descriptor_info(int index) {
    return descriptor_info(alignment_size, index * alignment_size);
}

/**
 * Invalidate a memory range of the buffer to make it visible to the host
 *
 * @note Only required for non-coherent memory
 */
void Buffer::invalidate_index(int index) {
    return invalidate(alignment_size, index * alignment_size);
}
