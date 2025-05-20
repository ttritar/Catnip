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
            setLayoutBindings.push_back(binding.second);
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
        m_PoolSizes.push_back({ descriptorType, count });
		return this;
    }


//============================================
// DESCRIPTOR SET
//============================================

    DescriptorSet::DescriptorSet(Device& device, UniformBuffer& ubo, std::vector<Image*> images, DescriptorSetLayout& setLayout, DescriptorPool& pool)
		: m_Device(device), m_DescriptorSetLayout(setLayout), m_DescriptorPool(pool)
	{
        std::vector<VkDescriptorSetLayout> layouts(cat::MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout.GetDescriptorSetLayout());
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool.GetDescriptorPool();
        allocInfo.descriptorSetCount = static_cast<uint32_t>(cat::MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        m_DescriptorSets.resize(cat::MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(m_Device.GetDevice(), &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < cat::MAX_FRAMES_IN_FLIGHT; i++)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = ubo.GetUniformBuffer(i);
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBuffer::UniformBufferObject);

           
            std::vector<VkWriteDescriptorSet> descriptorWrites{};

			VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = m_DescriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;
            descriptorWrite.pImageInfo = nullptr; //optional
            descriptorWrite.pTexelBufferView = nullptr; //optional
            descriptorWrites.emplace_back(descriptorWrite);

            for (int j{1}; j < images.size() + 1; j++)
            {
                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = images[j-1]->GetImageView();
                imageInfo.sampler = images[j-1]->GetSampler();


                descriptorWrite = {};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = m_DescriptorSets[i];
                descriptorWrite.dstBinding = j;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pImageInfo = &imageInfo;

				descriptorWrites.emplace_back(descriptorWrite);
            }


            vkUpdateDescriptorSets(m_Device.GetDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }

	}
}
