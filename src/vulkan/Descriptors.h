#pragma once

#include <unordered_map>

#include "Device.h"
#include "buffers/UniformBuffer.h"

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
		const VkDescriptorSetLayoutBinding& GetBinding(uint32_t binding) const
		{
			auto it = m_Bindings.find(binding);
			if (it != m_Bindings.end())
			{
				return it->second;
			}
			assert(false && "Binding not found in DescriptorSetLayout");
		}
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
		DescriptorSet(Device& device, DescriptorSetLayout& setLayout, DescriptorPool& pool, uint32_t count = MAX_FRAMES_IN_FLIGHT);

		DescriptorSet* UpdateAll();
		DescriptorSet* UpdateByIdx(uint32_t idx);
		DescriptorSet* AddBufferWrite(uint32_t binding, const std::vector<VkDescriptorBufferInfo>& bufferInfos);
		DescriptorSet* AddBufferWrite(uint32_t binding, const std::vector<VkDescriptorBufferInfo>& bufferInfos, uint32_t idx);
		DescriptorSet* AddImageWrite(uint32_t binding, const VkDescriptorImageInfo& imageInfo);
		DescriptorSet* AddImageWrite(uint32_t binding, const VkDescriptorImageInfo& imageInfo, uint32_t idx);

		void Bind(VkCommandBuffer commandBuffer, const VkPipelineLayout& pipelineLayout, uint16_t idx, unsigned int firstSet = 0) const;

		VkDescriptorSet* GetDescriptorSet(uint16_t idx) { return &m_DescriptorSets[idx]; }
		uint32_t GetDescriptorSetCount() const { return static_cast<uint32_t>(m_DescriptorSets.size()); }

	private:

		std::vector<VkDescriptorSet> m_DescriptorSets;

		std::vector<std::vector<VkWriteDescriptorSet>> m_DescriptorWrites;

		Device& m_Device;
		DescriptorSetLayout& m_DescriptorSetLayout;
		DescriptorPool& m_DescriptorPool;
	};


}