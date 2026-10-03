#include "context.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.hpp>

VulkanContext::VulkanContext() {
    instance = vk::createInstanceUnique(
        vk::InstanceCreateInfo({}, &app_info, layers));

    get_physical_device();

    float priority = 1.0f;
    vk::DeviceQueueCreateInfo queue_create_info({}, family_index, 1, &priority);
    device = physical_device->createDeviceUnique(vk::DeviceCreateInfo({}, queue_create_info));
    queue = device->getQueue(family_index, 0);

    command_pool =
        device->createCommandPoolUnique(vk::CommandPoolCreateInfo({}, family_index));
}

std::vector<vk::UniqueCommandBuffer> VulkanContext::create_command_buffers(std::size_t count) const {
    return device->allocateCommandBuffersUnique(
        vk::CommandBufferAllocateInfo(*command_pool, vk::CommandBufferLevel::ePrimary, count));
}

// init physical_device and compute_family_index
void VulkanContext::get_physical_device() {
    for (auto phys : instance->enumeratePhysicalDevices()) {
        auto qprops = phys.getQueueFamilyProperties();
        for (uint32_t i = 0; i < qprops.size(); ++i) {
            if (qprops[i].queueFlags & vk::QueueFlagBits::eCompute) {
                physical_device = vk::UniquePhysicalDevice(phys);
                family_index = i;
                return;
            }
        }
    }
    throw std::runtime_error("No compute-capable device found");
}

void VulkanContext::submit_command_buffer(const vk::UniqueCommandBuffer& command_buffer) const {
    vk::UniqueFence fence = device->createFenceUnique({});
    queue.submit(vk::SubmitInfo({}, {}, *command_buffer), *fence);
    if (device->waitForFences(*fence, vk::True, UINT64_MAX) != vk::Result::eSuccess)
        throw std::runtime_error("waitForFences failed");
}

void VulkanContext::copy_buffer(vk::UniqueBuffer& dst, const vk::UniqueBuffer& src, vk::DeviceSize len, vk::DeviceSize src_offset, vk::DeviceSize dst_offset) const {
    vk::UniqueCommandBuffer cmd = std::move(create_command_buffers(1)[0]);
    cmd->begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
    vk::BufferCopy cpy(src_offset, dst_offset, len);
    cmd->copyBuffer(*src, *dst, 1, &cpy);
    cmd->end();
    submit_command_buffer(cmd);
}
