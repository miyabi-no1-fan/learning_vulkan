#pragma once
#include <unordered_map>
#include <variant>
#include <vector>

#include "context.hpp"

class DescriptorSetLayout {
   public:
    class Builder {
       public:
        Builder(VulkanContext& ctx) : ctx{ ctx } {}

        Builder& addBinding(
            uint32_t binding,
            vk::DescriptorType descriptorType,
            vk::ShaderStageFlags stageFlags,
            uint32_t count = 1);
        DescriptorSetLayout build() const;

       private:
        const VulkanContext& ctx;
        std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings{};
    };

    DescriptorSetLayout(const VulkanContext& ctx, const std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding>& bindings);
    DescriptorSetLayout(const DescriptorSetLayout&) = delete;
    DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

    const vk::UniqueDescriptorSetLayout& getDescriptorSetLayout() const { return descriptorSetLayout; }

   private:
    vk::UniqueDescriptorSetLayout descriptorSetLayout;
    std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding> bindings;

    friend class DescriptorWriter;
};

class DescriptorPool {
   public:
    class Builder {
       public:
        Builder(const VulkanContext& ctx) : ctx{ ctx } {}

        Builder& addPoolSize(vk::DescriptorType descriptorType, uint32_t count);
        Builder& setPoolFlags(vk::DescriptorPoolCreateFlags flags);
        Builder& setMaxSets(uint32_t count);
        DescriptorPool build() const;

       private:
        const VulkanContext& ctx;
        std::vector<vk::DescriptorPoolSize> poolSizes{};
        uint32_t maxSets = 1000;
        vk::DescriptorPoolCreateFlags poolFlags = {};
    };

    DescriptorPool(
        const VulkanContext& ctx,
        uint32_t maxSets,
        vk::DescriptorPoolCreateFlags poolFlags,
        const std::vector<vk::DescriptorPoolSize>& poolSizes);
    DescriptorPool(const DescriptorPool&) = delete;
    DescriptorPool& operator=(const DescriptorPool&) = delete;

    vk::UniqueDescriptorSet allocateDescriptorSet(const vk::UniqueDescriptorSetLayout& descriptorSetLayout) const;
    void freeDescriptorSet(const std::vector<vk::DescriptorSet>& descriptors) const;
    void resetPool();

   private:
    const VulkanContext& ctx;
    vk::UniqueDescriptorPool descriptorPool;

    friend class DescriptorWriter;
};

class DescriptorWriter {
   public:
    DescriptorWriter(const VulkanContext& ctx, DescriptorSetLayout& setLayout, DescriptorPool& pool);

    DescriptorWriter& writeBuffer(uint32_t binding, vk::DescriptorBufferInfo bufferInfo);
    DescriptorWriter& writeImage(uint32_t binding, vk::DescriptorImageInfo imageInfo);

    vk::UniqueDescriptorSet build();
    void overwrite(const vk::UniqueDescriptorSet& set);

   private:
    const VulkanContext& ctx;
    DescriptorSetLayout& setLayout;
    DescriptorPool& pool;

    struct WriteInfo {
        uint32_t binding;
        std::variant<vk::DescriptorBufferInfo, vk::DescriptorImageInfo> descriptorInfo;
    };
    std::vector<WriteInfo> writeInfos;
};
