#include "descriptors.h"

#include <cassert>
#include <stdexcept>

namespace vre
{
    // *************** Descriptor Set Layout Builder *********************

    VreDescriptorSetLayout::Builder& VreDescriptorSetLayout::Builder::addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count)
    {
        assert(mBindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = descriptorType;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;
        mBindings[binding] = layoutBinding;
        return *this;
    }

    std::unique_ptr<VreDescriptorSetLayout> VreDescriptorSetLayout::Builder::build() const
    {
        return std::make_unique<VreDescriptorSetLayout>(mVreDevice, mBindings);
    }

    // *************** Descriptor Set Layout *********************

    VreDescriptorSetLayout::VreDescriptorSetLayout(VreDevice& device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : mVreDevice{ device }, mBindings{ bindings }
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto& kv : bindings)
        {
            setLayoutBindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
        descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(
            device.device(),
            &descriptorSetLayoutInfo,
            nullptr,
            &mDescriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    VreDescriptorSetLayout::~VreDescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(mVreDevice.device(), mDescriptorSetLayout, nullptr);
    }

    // *************** Descriptor Pool Builder *********************

    VreDescriptorPool::Builder& VreDescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType, uint32_t count)
    {
        poolSizes.push_back({ descriptorType, count });
        return *this;
    }

    VreDescriptorPool::Builder& VreDescriptorPool::Builder::setPoolFlags(
        VkDescriptorPoolCreateFlags flags)
    {
        poolFlags = flags;
        return *this;
    }
    VreDescriptorPool::Builder& VreDescriptorPool::Builder::setMaxSets(uint32_t count)
    {
        maxSets = count;
        return *this;
    }

    std::unique_ptr<VreDescriptorPool> VreDescriptorPool::Builder::build() const
    {
        return std::make_unique<VreDescriptorPool>(mVreDevice, maxSets, poolFlags, poolSizes);
    }

    // *************** Descriptor Pool *********************

    VreDescriptorPool::VreDescriptorPool(
        VreDevice& device,
        uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize>& poolSizes)
        : mVreDevice{ device }
    {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        descriptorPoolInfo.pPoolSizes = poolSizes.data();
        descriptorPoolInfo.maxSets = maxSets;
        descriptorPoolInfo.flags = poolFlags;

        if (vkCreateDescriptorPool(device.device(), &descriptorPoolInfo, nullptr, &mDescriptorPool) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    VreDescriptorPool::~VreDescriptorPool()
    {
        vkDestroyDescriptorPool(mVreDevice.device(), mDescriptorPool, nullptr);
    }

    bool VreDescriptorPool::allocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const
    {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = mDescriptorPool;
        allocInfo.pSetLayouts = &descriptorSetLayout;
        allocInfo.descriptorSetCount = 1;

        // Todo: Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up, see https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/
        if (vkAllocateDescriptorSets(mVreDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS)
            return false;

        return true;
    }

    void VreDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet>& descriptors) const
    {
        vkFreeDescriptorSets(
            mVreDevice.device(),
            mDescriptorPool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    void VreDescriptorPool::resetPool()
    {
        vkResetDescriptorPool(mVreDevice.device(), mDescriptorPool, 0);
    }

    // *************** Descriptor Writer *********************

    VreDescriptorWriter::VreDescriptorWriter(VreDescriptorSetLayout& setLayout, VreDescriptorPool& pool)
        : mSetLayout{ setLayout }, mPool{ pool }
    {
    }

    VreDescriptorWriter& VreDescriptorWriter::writeBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo)
    {
        assert(mSetLayout.mBindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = mSetLayout.mBindings[binding];

        assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pBufferInfo = bufferInfo;
        write.descriptorCount = 1;

        mWrites.push_back(write);
        return *this;
    }

    VreDescriptorWriter& VreDescriptorWriter::writeImage(uint32_t binding, VkDescriptorImageInfo* imageInfo)
    {
        assert(mSetLayout.mBindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto& bindingDescription = mSetLayout.mBindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.descriptorType = bindingDescription.descriptorType;
        write.dstBinding = binding;
        write.pImageInfo = imageInfo;
        write.descriptorCount = 1;

        mWrites.push_back(write);
        return *this;
    }

    bool VreDescriptorWriter::build(VkDescriptorSet& set)
    {
        if (!mPool.allocateDescriptorSet(mSetLayout.descriptorSetLayout(), set))
            return false;

        overwrite(set);
        return true;
    }

    void VreDescriptorWriter::overwrite(VkDescriptorSet& set)
    {
        for (auto& write : mWrites)
        {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(
            mPool.mVreDevice.device(), 
            static_cast<uint32_t>(mWrites.size()), 
            mWrites.data(), 
            0, 
            nullptr);
    }

}  // namespace vre