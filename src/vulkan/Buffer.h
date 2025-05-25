#pragma once

#include "Device.h"

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
		Buffer(Device& device,BufferInfo bufferInfo);
		~Buffer();


		// Methods
		//--------------------
		VkResult Map(VkDeviceSize offset = 0);
		void Unmap();
		void WriteToBuffer(void* data, VkDeviceSize size) const;

		// Getters & Setters
		VkBuffer GetBuffer() const { return m_Buffer; }

		void Flush();
		VmaAllocation GetAllocation() const { return m_Allocation; }
		VmaAllocationInfo GetAllocationInfo() const { return m_AllocationInfo; }
		void* GetRawData() const { return m_Mapped; }

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

		VmaAllocation m_Allocation = VK_NULL_HANDLE;
		VmaAllocationInfo m_AllocationInfo{};


	};
}
 