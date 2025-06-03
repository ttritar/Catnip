#include "Descriptors.h"

#include <array>
#include <stdexcept>

#include "SwapChain.h"

namespace cat
{
//============================================
// DESCRIPTOR SET LAYOUT
//============================================

	DescriptorSetLayout::DescriptorSetLayout(Device& device)
		: m_Device(device)
	{
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(m_Device.GetDevice(), m_DescriptorSetLayout, nullptr);
	}

	DescriptorSetLayout* DescriptorSetLayout::Create()
	{
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto& binding : m_Bindings) 
        {
            setLayoutBindings.emplace_back(binding.second);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(m_Bindings.size());
        layoutInfo.pBindings = setLayoutBindings.data();

        if (vkCreateDescriptorSetLayout(m_Device.GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

		return this;
	}

    DescriptorSetLayout* DescriptorSetLayout::AddBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t count)
    {
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding;
        layoutBinding.descriptorType = type;
        layoutBinding.descriptorCount = count;
        layoutBinding.stageFlags = stageFlags;

        m_Bindings[binding] = layoutBinding;

		return this;
    }


//============================================
// DESCRIPTOR POOL
//============================================

	DescriptorPool::DescriptorPool(Device& device)
		: m_Device(device)
	{

	}

	DescriptorPool::~DescriptorPool()
	{
		vkDestroyDescriptorPool(m_Device.GetDevice(), m_DescriptorPool, nullptr);
	}

	DescriptorPool* DescriptorPool::Create(uint32_t maxSets)
	{
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(m_PoolSizes.size());
        poolInfo.pPoolSizes = m_PoolSizes.data();
        poolInfo.maxSets = maxSets;

        if (vkCreateDescriptorPool(m_Device.GetDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }

		return this;
	}

    DescriptorPool* DescriptorPool::AddPoolSize(VkDescriptorType descriptorType, uint32_t count)
    {
        m_PoolSizes.emplace_back(descriptorType, count);
		return this;
    }


//============================================
// DESCRIPTOR SET
//============================================

    DescriptorSet::DescriptorSet(Device& device, DescriptorSetLayout& setLayout, DescriptorPool& pool, uint32_t count)
		:m_Device(device),m_DescriptorSetLayout(setLayout), m_DescriptorPool(pool)
	{
        std::vector<VkDescriptorSetLayout> layouts(count, m_DescriptorSetLayout.GetDescriptorSetLayout());

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool.GetDescriptorPool();
        allocInfo.descriptorSetCount = count;
        allocInfo.pSetLayouts = layouts.data();

        m_DescriptorSets.resize(count);
		m_DescriptorWrites.resize(count);

        const auto result = vkAllocateDescriptorSets(m_Device.GetDevice(), &allocInfo, m_DescriptorSets.data());
        if (result == VK_ERROR_OUT_OF_POOL_MEMORY)
        {
            throw std::runtime_error("No more memory in pool! :( verry sad");
        }
	}

    DescriptorSet* DescriptorSet::AddBufferWrite(uint32_t binding, const std::vector<VkDescriptorBufferInfo>& bufferInfos)
    {
		const auto& bindingDesc = m_DescriptorSetLayout.GetBinding(binding);

        for (size_t i = 0; i < m_DescriptorSets.size(); ++i)
        {
            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = m_DescriptorSets[i];
            descriptorWrite.dstBinding = binding;
            descriptorWrite.descriptorType = bindingDesc.descriptorType;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfos[i];

            m_DescriptorWrites[i].emplace_back(descriptorWrite);
        }
        return this;
    }
    DescriptorSet* DescriptorSet::AddBufferWrite(uint32_t binding, const std::vector<VkDescriptorBufferInfo>& bufferInfos, uint32_t idx)
    {
        const auto& bindingDesc = m_DescriptorSetLayout.GetBinding(binding);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSets[idx];
        descriptorWrite.dstBinding = binding;
        descriptorWrite.descriptorType = bindingDesc.descriptorType;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfos[idx];

        m_DescriptorWrites[idx].emplace_back(descriptorWrite);

        return this;
    }

    DescriptorSet* DescriptorSet::AddImageWrite(uint32_t binding, const VkDescriptorImageInfo& imageInfo)
    {
		const auto& bindingDesc = m_DescriptorSetLayout.GetBinding(binding);

        for (size_t i = 0; i < m_DescriptorSets.size(); ++i)
        {
            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite = {};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = m_DescriptorSets[i];
            descriptorWrite.dstBinding = binding;
            descriptorWrite.descriptorType = bindingDesc.descriptorType;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pImageInfo = &imageInfo;

            m_DescriptorWrites[i].emplace_back(descriptorWrite);
        }
		return this;
    }
    DescriptorSet* DescriptorSet::AddImageWrite(uint32_t binding, const VkDescriptorImageInfo& imageInfo, uint32_t idx)
    {
        const auto& bindingDesc = m_DescriptorSetLayout.GetBinding(binding);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_DescriptorSets[idx];
        descriptorWrite.dstBinding = binding;
        descriptorWrite.descriptorType = bindingDesc.descriptorType;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        m_DescriptorWrites[idx].emplace_back(descriptorWrite);
       
        return this;
    }

    void DescriptorSet::Bind(VkCommandBuffer commandBuffer, const VkPipelineLayout& pipelineLayout, uint16_t idx, unsigned int firstSet) const
    {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, firstSet, 1, &m_DescriptorSets[idx], 0, nullptr);
    }

    DescriptorSet* DescriptorSet::UpdateAll()
    {
		for (uint32_t i = 0; i < m_DescriptorSets.size(); ++i)
		{
			UpdateByIdx(i);
		}

		return this;
    }
    DescriptorSet* DescriptorSet::UpdateByIdx(uint32_t idx)
    {
        vkUpdateDescriptorSets(m_Device.GetDevice(), static_cast<uint32_t>(m_DescriptorWrites[idx].size()), m_DescriptorWrites[idx].data(), 0, nullptr);
        return this;
    }
}
