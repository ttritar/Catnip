#include "Buffer.h"

#include <stdexcept>
#include <cassert>

namespace cat
{
    // CTOR & DTOR
    //--------------------
    Buffer::Buffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
		: m_Device{ device }, m_BufferSize{size}
    {
		m_Device.CreateBuffer(size, usage, properties, m_Buffer, m_BufferMemory);
    }

    Buffer::~Buffer()
    {
        Unmap();
		vkDestroyBuffer(m_Device.GetDevice(), m_Buffer, nullptr);
		vkFreeMemory(m_Device.GetDevice(), m_BufferMemory, nullptr);
    }



    VkResult Buffer::Map(VkDeviceSize offset)
    {
        assert(m_Buffer && m_BufferMemory && "Called map on buffer before create");
        return vkMapMemory(m_Device.GetDevice(), m_BufferMemory, offset, m_BufferSize, 0, &m_Mapped);
    }

    void Buffer::Unmap()
    {
        if (m_Mapped)
        {
			vkUnmapMemory(m_Device.GetDevice(), m_BufferMemory);
			m_Mapped = nullptr;
        }
    }

    void Buffer::WriteToBuffer(void* data,VkDeviceSize offset) const
    {
        assert(m_Mapped && "Cannot copy to unmapped buffer");

        if (m_BufferSize == VK_WHOLE_SIZE)
        {
            memcpy(m_Mapped, data, m_BufferSize);
        }
        else 
        {
            char* memOffset = (char*)m_Mapped;
            memOffset += offset;
            memcpy(memOffset, data, m_BufferSize);
        }
    }


    // Creators
    //--------------------

	


	// Helpers
    //--------------------

}
