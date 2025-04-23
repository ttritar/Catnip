#include "Model.h"

namespace cat
{
	// CTOR & DTOR
	//--------------------
    Model::Model(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
		: m_Device{ device }, m_Vertices{ vertices }, m_Indices{ indices }
	{
        CreateVertexBuffer();
		CreateIndexBuffer();
	}

	Model::~Model()
    {
		vkDestroyBuffer(m_Device.GetDevice(), m_IndexBuffer, nullptr);
		vkFreeMemory(m_Device.GetDevice(), m_IndexBufferMemory, nullptr);

		vkDestroyBuffer(m_Device.GetDevice(), m_VertexBuffer, nullptr);
		vkFreeMemory(m_Device.GetDevice(), m_VertexBufferMemory, nullptr);
    }

    // Creators
	//--------------------
    void Model::CreateVertexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();
    
        // USING THE STAGING BUFFER
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);
    
    
        // MAPPING THE BUFFER MEMORY
        void* data;
        vkMapMemory(m_Device.GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, m_Vertices.data(), (size_t)bufferSize);
        vkUnmapMemory(m_Device.GetDevice(), stagingBufferMemory);
    
    
        m_Device.CreateBuffer(bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);
    
        m_Device.CopyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);
    
        vkDestroyBuffer(m_Device.GetDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_Device.GetDevice(), stagingBufferMemory, nullptr);
    }

    void Model::CreateIndexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(m_Device.GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, m_Indices.data(), (size_t)bufferSize);
        vkUnmapMemory(m_Device.GetDevice(), stagingBufferMemory);

        m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);

        copyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);

        vkDestroyBuffer(m_Device.GetDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_Device.GetDevice(), stagingBufferMemory, nullptr);
    }



	// Helpers
	//--------------------

}

