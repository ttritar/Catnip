#include "Mesh.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "Image.h"

namespace std
{
    template<> struct hash<cat::Mesh::Vertex>
	{
        size_t operator()(cat::Mesh::Vertex const& vertex) const
    	{
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.uv) << 1);
        }
    };
}

namespace cat
{
    // CTOR & DTOR
    //--------------------
	Mesh::Mesh(Device& device, SwapChain& swapchain, UniformBuffer* ubo, DescriptorSetLayout* layout, DescriptorPool* pool,
        const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const Material &material)
		: m_Device{ device }, m_Vertices{ vertices }, m_Indices{ indices }
    {
        CreateVertexBuffer();
        CreateIndexBuffer();

    	m_Images.push_back(new Image(device, swapchain, material.diffusePath.c_str()));
        m_pDescriptorSet = new DescriptorSet(device, *ubo, m_Images, *layout, *pool);
    } 

    Mesh::~Mesh()
    {
		delete m_pDescriptorSet;
		m_pDescriptorSet = nullptr;

	    for (auto& image : m_Images)
	    {
            delete image;
			image = nullptr;
	    }

        delete m_IndexBuffer;
        m_IndexBuffer = nullptr;

		delete m_VertexBuffer;
        m_VertexBuffer = nullptr;

    }


    void Mesh::Draw(VkCommandBuffer commandBuffer)
	{
		if(m_HasIndexBuffer)
			vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, 0, 0, 0);
		else
			vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
    }

    void Mesh::Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t idx)
    {
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0,
            1,
            m_pDescriptorSet->GetDescriptorSet(idx),
            0,
            nullptr
        );

        VkBuffer buffers[] = { m_VertexBuffer->GetBuffer() };
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        if (m_HasIndexBuffer) 
            vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

    }


    // Creators
    //--------------------
    void Mesh::CreateVertexBuffer()
    {
		m_VertexBufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();
		m_VertexCount = m_Vertices.size();

        // USING THE STAGING BUFFER
        Buffer stagingBuffer{
         m_Device, m_VertexBufferSize,
         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.Map();
		stagingBuffer.WriteToBuffer((void*)m_Vertices.data());

        m_VertexBuffer = new Buffer(
            m_Device, m_VertexBufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        m_Device.CopyBuffer(stagingBuffer.GetBuffer(), m_VertexBuffer->GetBuffer(), m_VertexBufferSize);
    }

    void Mesh::CreateIndexBuffer()
    {
        m_HasIndexBuffer = static_cast<uint32_t>(m_Indices.size()) > 0;

        if (!m_HasIndexBuffer) return;

    	m_IndexBufferSize = sizeof(m_Indices[0]) * m_Indices.size();
        m_IndexCount = m_Indices.size();

        Buffer stagingBuffer{
            m_Device, m_IndexBufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer((void*)m_Indices.data());

        m_IndexBuffer = new Buffer(
            m_Device, m_IndexBufferSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_Device.CopyBuffer(stagingBuffer.GetBuffer(), m_IndexBuffer->GetBuffer(), m_IndexBufferSize);
    }
}
