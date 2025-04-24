#pragma once

#include "Device.h"

namespace cat
{
	class DescriptorSetLayout final
	{
	public:
		// CTOR & DTOR
		//--------------------
		DescriptorSetLayout(Device& devic);
		~DescriptorSetLayout();

		DescriptorSetLayout(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;
		DescriptorSetLayout(DescriptorSetLayout&&) = delete;
		DescriptorSetLayout& operator=(DescriptorSetLayout&&) = delete;

		VkDescriptorSetLayout GetDescriptorSetLayout() const { return m_DescriptorSetLayout; }
	private:
		VkDescriptorSetLayout m_DescriptorSetLayout;

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

		VkDescriptorPool GetDescriptorPool() const { return m_DescriptorPool; }
	private:
		VkDescriptorPool m_DescriptorPool;

		Device& m_Device;
	};

	class DescriptorSet final
	{
	public:
		DescriptorSet(Device& device, DescriptorSetLayout& setLayout, DescriptorPool& pool);
		~DescriptorSet();

		DescriptorSet(const DescriptorSet&) = delete;
		DescriptorSet& operator=(const DescriptorSet&) = delete;
		DescriptorSet(DescriptorSet&&) = delete;
		DescriptorSet& operator=(DescriptorSet&&) = delete;

	private:
		std::vector<VkDescriptorSet> m_DescriptorSets;

		Device& m_Device;
		DescriptorSetLayout& m_DescriptorSetLayout;
		DescriptorPool& m_DescriptorPool;
	};


}