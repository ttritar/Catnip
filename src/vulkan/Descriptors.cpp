#include "Descriptors.h"

#include <array>
#include "SwapChain.h"

namespace cat
{
//============================================
// DESCRIPTOR SET LAYOUT
//============================================

	DescriptorSetLayout::DescriptorSetLayout(Device& device)
		: m_Device(device)
	{
		// Making a UBO layout binding
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;   //which shader stages the descriptor is going to be referenced

		// Making the sampler layout bindings -> TODO: MAKE THIS SCALABLE
        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding,samplerLayoutBinding };
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(m_Device.GetDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
	}

	DescriptorSetLayout::~DescriptorSetLayout()
	{
		vkDestroyDescriptorSetLayout(m_Device.GetDevice(), m_DescriptorSetLayout, nullptr);
	}


//============================================
// DESCRIPTOR POOL
//============================================

	DescriptorPool::DescriptorPool(Device& device)
		: m_Device(device)
	{
		// Making the pool -> TODO: MAKE THIS SCALABLE
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(cat::MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(cat::MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(cat::MAX_FRAMES_IN_FLIGHT);

        if (vkCreateDescriptorPool(m_Device.GetDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
	}

	DescriptorPool::~DescriptorPool()
	{
		vkDestroyDescriptorPool(m_Device.GetDevice(), m_DescriptorPool, nullptr);
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

            for (int j{1}; j <images.size() + 1; j++)
            {
                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = images[i]->GetTextureImageView();
                imageInfo.sampler = images[i]->GetTextureSampler();


                descriptorWrite = {};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = m_DescriptorSets[i];
                descriptorWrite.dstBinding = 1;
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
