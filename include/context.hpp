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

    // void f(vk::UniqueCommandBuffer& cmd);
    template <typename F>
    void one_time_submit(F&& f) const {
        vk::UniqueCommandBuffer cmd = std::move(create_command_buffers(1)[0]);
        cmd->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
        (void)f(cmd);
        cmd->end();
        submit_command_buffer(cmd);
    }

   private:
    void get_physical_device();
};
