#pragma once

#include <unordered_map>

#include "Device.h"
#include "buffers/UniformBuffer.h"
#include "scene/Image.h"

namespace cat
{
	class DescriptorSetLayout final
	{
	public:
		// CTOR & DTOR
		//--------------------
		DescriptorSetLayout(Device& device);
		~DescriptorSetLayout();

		DescriptorSetLayout(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout(DescriptorSetLayout&&) = delete;
		DescriptorSetLayout& operator=(DescriptorSetLayout&&) = delete;

		DescriptorSetLayout* Create();
		DescriptorSetLayout* AddBinding(uint32_t binding, VkDescriptorType type, VkShaderStageFlags stageFlags, uint32_t count=1);

		VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }
	private:
		VkDescriptorSetLayout m_DescriptorSetLayout;
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> m_Bindings;

		Device& m_Device;
	};

	class DescriptorPool final
	{
	public:
		// CTOR & DTOR
		//--------------------
		DescriptorPool(Device& device);
		~DescriptorPool();

		DescriptorPool(const DescriptorPool&) = delete;
		DescriptorPool& operator=(const DescriptorPool&) = delete;
		DescriptorPool(DescriptorPool&&) = delete;
		DescriptorPool& operator=(DescriptorPool&&) = delete;

		DescriptorPool* Create(uint32_t maxSets);
		DescriptorPool* AddPoolSize(VkDescriptorType descriptorType, uint32_t count);

		VkDescriptorPool GetDescriptorPool() const { return m_DescriptorPool; }

	private:
		VkDescriptorPool m_DescriptorPool;
		std::vector<VkDescriptorPoolSize> m_PoolSizes;

		Device& m_Device;
	};

	class DescriptorSet final
	{
	public:
		DescriptorSet(Device& device, UniformBuffer& ubo, std::vector<Image*> images, DescriptorSetLayout& setLayout, DescriptorPool& pool);

		VkDescriptorSet* GetDescriptorSet(uint16_t idx) { return &m_DescriptorSets[idx]; }
		void Bind(VkCommandBuffer commandBuffer, const VkPipelineLayout& pipelineLayout, uint16_t idx) const
		{
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &m_DescriptorSets[idx], 0, nullptr);
		}

	private:
		std::vector<VkDescriptorSet> m_DescriptorSets;

		Device& m_Device;
		DescriptorSetLayout& m_DescriptorSetLayout;
		DescriptorPool& m_DescriptorPool;
	};


}