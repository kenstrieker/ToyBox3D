#pragma once
#include "device.hpp"
#include <memory>
#include <unordered_map>
#include <vector>

namespace engine {
    class descriptorSetLayout {
    public:
        class Builder {
        public:
            Builder(device& deviceInstance) : deviceInstance{ deviceInstance } {}
            Builder& addBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count = 1);
            std::unique_ptr<descriptorSetLayout> build() const;

        private:
            device& deviceInstance;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings = {};
        };

        descriptorSetLayout(device& deviceInstance, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~descriptorSetLayout();

        descriptorSetLayout(const descriptorSetLayout&) = delete;
        descriptorSetLayout& operator=(const descriptorSetLayout&) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayoutInstance; }

    private:
        device& deviceInstance;
        VkDescriptorSetLayout descriptorSetLayoutInstance;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class descriptorWriter;
    };

    class descriptorPool {
    public:
        class Builder {
        public:
            Builder(device& deviceInstance) : deviceInstance{ deviceInstance } {}

            Builder& addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder& setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder& setMaxSets(uint32_t count);
            std::unique_ptr<descriptorPool> build() const;

        private:
            device& deviceInstance;
            std::vector<VkDescriptorPoolSize> poolSizes = {};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        descriptorPool(device& deviceInstance, uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes);
        ~descriptorPool();

        descriptorPool(const descriptorPool&) = delete;
        descriptorPool& operator=(const descriptorPool&) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const;

        void resetPool();

    private:
        device& deviceInstance;
        VkDescriptorPool descriptorPoolInstance;

        friend class descriptorWriter;
    };

    class descriptorWriter {
    public:
        descriptorWriter(descriptorSetLayout& setLayout, descriptorPool& pool);

        descriptorWriter& writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
        descriptorWriter& writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);

        bool build(VkDescriptorSet& set);
        void overwrite(VkDescriptorSet& set);

    private:
        descriptorSetLayout& setLayout;
        descriptorPool& pool;
        std::vector<VkWriteDescriptorSet> writes;
    };
}