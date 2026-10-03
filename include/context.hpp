#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.hpp>

class VulkanContext {
   private:
    static constexpr const char* layers[] = { "VK_LAYER_KHRONOS_validation" };

   public:
    vk::ApplicationInfo app_info{
        nullptr,
        VK_MAKE_VERSION(1, 0, 0),
        nullptr,
        VK_MAKE_VERSION(1, 0, 0),
        VK_API_VERSION_1_1,
    };
    vk::UniqueInstance instance;
    vk::UniquePhysicalDevice physical_device;
    std::uint32_t family_index;
    vk::UniqueDevice device;
    vk::Queue queue;

    vk::UniqueCommandPool command_pool;

    VulkanContext();

    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;
    VulkanContext(VulkanContext&&) noexcept = default;
    VulkanContext& operator=(VulkanContext&&) noexcept = default;

    std::vector<vk::UniqueCommandBuffer> create_command_buffers(std::size_t count) const;
    void submit_command_buffer(const vk::UniqueCommandBuffer& command_buffer) const;

    void copy_buffer(vk::UniqueBuffer& dst, const vk::UniqueBuffer& src, vk::DeviceSize len, vk::DeviceSize src_offset = 0, vk::DeviceSize dst_offset = 0) const;

   private:
    void get_physical_device();
};
