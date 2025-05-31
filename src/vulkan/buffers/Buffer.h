#pragma once

#include "../Device.h"

namespace cat
{
	class Buffer final
	{
	public:
		struct BufferInfo
		{
			VkDeviceSize size;
			VkBufferUsageFlags usageFlags;
			VmaMemoryUsage memoryUsage;
			bool mappable = true;
		};

		// CTOR & DTOR
		//--------------------
		Buffer(Device& device,const BufferInfo& bufferInfo);
		~Buffer();


		// Methods
		//--------------------
		VkResult Map();
		void Unmap();
		void WriteToBuffer(void* data) const;

		// Getters & Setters
		VkBuffer GetBuffer() const { return m_Buffer; }

		void Flush();
		VmaAllocation GetAllocation() const { return m_Allocation; }
		VmaAllocationInfo GetAllocationInfo() const { return m_AllocationInfo; }
		void* GetRawData() const { return m_Mapped; }

		VkDescriptorBufferInfo GetDescriptorBufferInfo() const
		{
			return VkDescriptorBufferInfo{
				.buffer = m_Buffer,
				.offset = 0,
				.range = m_Size
			};
		}

	private:
		// Private Methods
		//--------------------

		// Creators


		// Helpers


		// Private Members
		//--------------------
		Device& m_Device;
		VkBuffer m_Buffer;
		void* m_Mapped = nullptr;

		VkDeviceSize m_Offset = 0;
		VkDeviceSize m_Size = VK_WHOLE_SIZE;

		VmaAllocation m_Allocation = VK_NULL_HANDLE;
		VmaAllocationInfo m_AllocationInfo{};


	};
}
 