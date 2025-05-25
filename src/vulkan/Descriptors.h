#pragma once

#include <unordered_map>

#include "Device.h"
#include "buffers/UniformBuffer.h"
#include "Image.h"

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

	private:
		std::vector<VkDescriptorSet> m_DescriptorSets;

		Device& m_Device;
		DescriptorSetLayout& m_DescriptorSetLayout;
		DescriptorPool& m_DescriptorPool;
	};


}