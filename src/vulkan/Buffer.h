#pragma once

#include "Device.h"

namespace cat
{
	class Buffer final
	{
	public:
		// CTOR & DTOR
		//--------------------
		Buffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		~Buffer();


		// Methods
		//--------------------
		VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
		void Unmap();
		void WriteToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

		// Getters & Setters
		VkBuffer GetBuffer() const { return m_Buffer; }
		VkDeviceMemory GetBufferMemory() const { return m_BufferMemory; }

	private:
		// Private Methods
		//--------------------

		// Creators


		// Helpers


		// Private Members
		//--------------------
		VkBuffer m_Buffer;
		VkDeviceMemory m_BufferMemory;
		VkDeviceSize m_BufferSize;
		void* m_Mapped = nullptr;

		Device& m_Device;

	};
}
 