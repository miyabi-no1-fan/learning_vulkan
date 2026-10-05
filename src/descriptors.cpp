#include "descriptors.hpp"

#include <cassert>
#include <variant>
#include <vector>

// *************** Descriptor Set Layout Builder *********************

DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::addBinding(
    uint32_t binding,
    vk::DescriptorType descriptorType,
    vk::ShaderStageFlags stageFlags,
    uint32_t count) {
    assert(bindings.count(binding) == 0 && "Binding already in use");
    vk::DescriptorSetLayoutBinding layoutBinding{};
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = descriptorType;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stageFlags;
    bindings[binding] = layoutBinding;
    return *this;
}

DescriptorSetLayout DescriptorSetLayout::Builder::build() const {
    return DescriptorSetLayout(ctx, bindings);
}

// *************** Descriptor Set Layout *********************

DescriptorSetLayout::DescriptorSetLayout(const VulkanContext& ctx, const std::unordered_map<uint32_t, vk::DescriptorSetLayoutBinding>& bindings)
    : bindings{ bindings } {
    std::vector<vk::DescriptorSetLayoutBinding> setLayoutBindings{};
    setLayoutBindings.reserve(bindings.bucket_count());
    for (auto&& kv : bindings) setLayoutBindings.push_back(kv.second);
    descriptorSetLayout = ctx.device->createDescriptorSetLayoutUnique(vk::DescriptorSetLayoutCreateInfo({}, setLayoutBindings));
}

// *************** Descriptor Pool Builder *********************

DescriptorPool::Builder& DescriptorPool::Builder::addPoolSize(
    vk::DescriptorType descriptorType, uint32_t count) {
    poolSizes.push_back({ descriptorType, count });
    return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::setPoolFlags(
    vk::DescriptorPoolCreateFlags flags) {
    poolFlags = flags;
    return *this;
}
DescriptorPool::Builder& DescriptorPool::Builder::setMaxSets(uint32_t count) {
    maxSets = count;
    return *this;
}

DescriptorPool DescriptorPool::Builder::build() const {
    return DescriptorPool(ctx, maxSets, poolFlags, poolSizes);
}

// *************** Descriptor Pool *********************

DescriptorPool::DescriptorPool(
    const VulkanContext& ctx,
    uint32_t maxSets,
    vk::DescriptorPoolCreateFlags poolFlags,
    const std::vector<vk::DescriptorPoolSize>& poolSizes)
    : ctx{ ctx } {
    descriptorPool =
        ctx.device->createDescriptorPoolUnique(
            vk::DescriptorPoolCreateInfo(
                poolFlags | vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
                maxSets,
                static_cast<uint32_t>(poolSizes.size()),
                poolSizes.data()));
}

vk::UniqueDescriptorSet
DescriptorPool::allocateDescriptorSet(const vk::UniqueDescriptorSetLayout& descriptorSetLayout) const {
    return std::move(ctx.device->allocateDescriptorSetsUnique(
        vk::DescriptorSetAllocateInfo(
            *descriptorPool,
            *descriptorSetLayout))[0]);
}

void DescriptorPool::freeDescriptorSet(const std::vector<vk::DescriptorSet>& descriptors) const {
    ctx.device->freeDescriptorSets(*descriptorPool, descriptors);
}

void DescriptorPool::resetPool() {
    ctx.device->resetDescriptorPool(*descriptorPool);
}

// *************** Descriptor Writer *********************

DescriptorWriter::DescriptorWriter(const VulkanContext& ctx, DescriptorSetLayout& setLayout, DescriptorPool& pool)
    : ctx(ctx), setLayout{ setLayout }, pool{ pool } {}

DescriptorWriter& DescriptorWriter::writeBuffer(
    uint32_t binding, vk::DescriptorBufferInfo bufferInfo) {
    assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
    assert(setLayout.bindings[binding].descriptorCount == 1 &&
           "Binding single descriptor info, but binding expects multiple");
    writeInfos.push_back(WriteInfo{
        binding,
        bufferInfo,
    });
    return *this;
}

DescriptorWriter& DescriptorWriter::writeImage(
    uint32_t binding, vk::DescriptorImageInfo imageInfo) {
    assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
    assert(setLayout.bindings[binding].descriptorCount == 1 &&
           "Binding single descriptor info, but binding expects multiple");
    writeInfos.push_back(WriteInfo{
        binding,
        imageInfo,
    });
    return *this;
}

vk::UniqueDescriptorSet DescriptorWriter::build() {
    vk::UniqueDescriptorSet set = pool.allocateDescriptorSet(setLayout.getDescriptorSetLayout());
    overwrite(set);
    return set;
}

void DescriptorWriter::overwrite(const vk::UniqueDescriptorSet& set) {
    std::vector<vk::WriteDescriptorSet> writes{};
    for (auto&& info : writeInfos) {
        if (auto* imageInfo = std::get_if<vk::DescriptorImageInfo>(&info.descriptorInfo)) {
            writes.push_back(vk::WriteDescriptorSet(
                *set,
                info.binding,
                {},
                1,
                setLayout.bindings[info.binding].descriptorType,
                imageInfo,
                nullptr,
                nullptr));
        } else if (auto* bufferInfo = std::get_if<vk::DescriptorBufferInfo>(&info.descriptorInfo)) {
            writes.push_back(vk::WriteDescriptorSet(
                *set,
                info.binding,
                {},
                1,
                setLayout.bindings[info.binding].descriptorType,
                nullptr,
                bufferInfo,
                nullptr));
        }
    }
    ctx.device->updateDescriptorSets(writes, {});
}
