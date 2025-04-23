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


		Device& m_Device;
	};
}
 