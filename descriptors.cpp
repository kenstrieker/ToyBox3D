#include "descriptors.hpp"
#include <cassert>
#include <stdexcept>

namespace engine {

    // *************** Descriptor Set Layout Builder *********************

    descriptorSetLayout::Builder& descriptorSetLayout::Builder::addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count) {
        assert(bindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding = {};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        bindings[binding] = layoutBinding;
        return *this;
    }

    std::unique_ptr<descriptorSetLayout> descriptorSetLayout::Builder::build() const {
        return std::make_unique<descriptorSetLayout>(deviceInstance, bindings);
    }

    // *************** Descriptor Set Layout *********************

    descriptorSetLayout::descriptorSetLayout(device& deviceInstance, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings) : deviceInstance{ deviceInstance }, bindings{ bindings } {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {};
        for (auto kv : bindings) {
            setLayoutBindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(deviceInstance.getDevice(), &descriptorSetLayoutInfo, nullptr, &descriptorSetLayoutInstance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    descriptorSetLayout::~descriptorSetLayout() {
        vkDestroyDescriptorSetLayout(deviceInstance.getDevice(), descriptorSetLayoutInstance, nullptr);
    }

    // *************** Descriptor Pool Builder *********************

    descriptorPool::Builder& descriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count) {
        poolSizes.push_back({ descriptorType, count });
        return *this;
    }

    descriptorPool::Builder& descriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags) {
        poolFlags = flags;
        return *this;
    }
    descriptorPool::Builder& descriptorPool::Builder::setMaxSets(uint32_t count) {
        maxSets = count;
        return *this;
    }

    std::unique_ptr<descriptorPool> descriptorPool::Builder::build() const {
        return std::make_unique<descriptorPool>(deviceInstance, maxSets, poolFlags, poolSizes);
    }

    // *************** Descriptor Pool *********************

    descriptorPool::descriptorPool(device& deviceInstance, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes) : deviceInstance{ deviceInstance } {
        VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        if (vkCreateDescriptorPool(deviceInstance.getDevice(), &descriptorPoolInfo, nullptr, &descriptorPoolInstance) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    descriptorPool::~descriptorPool() {
        vkDestroyDescriptorPool(deviceInstance.getDevice(), descriptorPoolInstance, nullptr);
    }

    bool descriptorPool::allocateDescriptor(const VkDescriptorSetLayout descriptorSetLayoutInstance, VkDescriptorSet& descriptor) const {
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPoolInstance;
        allocInfo.pSetLayouts = &descriptorSetLayoutInstance;
        allocInfo.descriptorSetCount = 1;

        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        if (vkAllocateDescriptorSets(deviceInstance.getDevice(), &allocInfo, &descriptor) != VK_SUCCESS) {
            return false;
        }
        return true;
    }

    void descriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const {
        vkFreeDescriptorSets(deviceInstance.getDevice(), descriptorPoolInstance, static_cast<uint32_t>(descriptors.size()), descriptors.data());
    }

    void descriptorPool::resetPool() {
        vkResetDescriptorPool(deviceInstance.getDevice(), descriptorPoolInstance, 0);
    }

    // *************** Descriptor Writer *********************

    descriptorWriter::descriptorWriter(descriptorSetLayout& setLayout, descriptorPool& pool) : setLayout{ setLayout }, pool{ pool } {}

    descriptorWriter& descriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    descriptorWriter& descriptorWriter::writeImage(
        uint32_t binding, VkDescriptorImageInfo* imageInfo) {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = setLayout.bindings[binding];

        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        writes.push_back(write);
        return *this;
    }

    bool descriptorWriter::build(VkDescriptorSet& set) {
        bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
        if (!success) {
            return false;
        }
        overwrite(set);
        return true;
    }

    void descriptorWriter::overwrite(VkDescriptorSet& set) {
        for (auto& write : writes) {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool.deviceInstance.getDevice(), writes.size(), writes.data(), 0, nullptr);
    }
}