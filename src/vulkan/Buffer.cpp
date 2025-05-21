#include "Buffer.h"

#include <stdexcept>
#include <cassert>
#include <iostream>

namespace cat
{
    // CTOR & DTOR
    //--------------------
    Buffer::Buffer(Device& device, VkDeviceSize size, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, bool mappable)
		: m_Device{ device } 
    {
        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = memoryUsage;

        if (mappable) {
            allocInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
        }
        else {
            allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        }
        
        m_Device.CreateBuffer(size, usageFlags, memoryUsage, mappable, m_Buffer, m_Allocation);
    }

    Buffer::~Buffer()
    {
    	Unmap();
        
        vmaDestroyBuffer(m_Device.GetAllocator(), m_Buffer, m_Allocation);
    }

	//                             >^._.^<      <3

    VkResult Buffer::Map(VkDeviceSize offset)
    {
        return vmaMapMemory(m_Device.GetAllocator(), m_Allocation, &m_Mapped);
    }

    void Buffer::Unmap()
    {
        if (m_Mapped != nullptr) 
        {
            vmaUnmapMemory(m_Device.GetAllocator(), m_Allocation);
            m_Mapped = nullptr;
        }
    }

    void Buffer::WriteToBuffer(void* data,VkDeviceSize size) const
    {
        vmaCopyMemoryToAllocation(m_Device.GetAllocator(), data, m_Allocation, 0, size);
    }

    void Buffer::Flush()
	{
        vmaFlushAllocation(m_Device.GetAllocator(), m_Allocation, 0, VK_WHOLE_SIZE);
    }


    // Creators
    //--------------------

	


	// Helpers
    //--------------------

}
