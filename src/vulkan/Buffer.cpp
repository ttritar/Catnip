#include "Buffer.h"

#include <stdexcept>

namespace cat
{
    // CTOR & DTOR
    //--------------------
    Buffer::Buffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
		: m_Device{ device }
    {
		m_Device.CreateBuffer(size, usage, properties, m_Buffer, m_BufferMemory);
    }

    Buffer::~Buffer()
    {
    }


    // Creators
    //--------------------

	


	// Helpers
    //--------------------

}
