#include "Mesh.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h" 

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

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
    Mesh::Mesh(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices)
        : m_Device{ device }, m_Vertices{ vertices }, m_Indices{ indices }
    {
        CreateVertexBuffer();
        CreateIndexBuffer();
    }

    Mesh::Mesh(Device& device, const std::string& path)
		: m_Device{ device }
    {
        LoadObj(path);
        CreateVertexBuffer();
        CreateIndexBuffer();
    }

    Mesh::~Mesh()
    {
		delete m_VertexBuffer;
        m_VertexBuffer = nullptr;

		delete m_IndexBuffer;
        m_IndexBuffer = nullptr;
    }



    void Mesh::Draw(VkCommandBuffer commandBuffer)
	{
		if(m_HasIndexBuffer)
			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
		else
			vkCmdDraw(commandBuffer, static_cast<uint32_t>(m_Vertices.size()), 1, 0, 0);
    }

    void Mesh::Bind(VkCommandBuffer commandBuffer)
    {
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
        VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();
        uint32_t vertexSize = sizeof(m_Vertices[0]);

        // USING THE STAGING BUFFER
        Buffer stagingBuffer{
         m_Device, bufferSize,
         VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.Map();
		stagingBuffer.WriteToBuffer((void*)m_Vertices.data());

        m_VertexBuffer = new Buffer(
            m_Device, bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        m_Device.CopyBuffer(stagingBuffer.GetBuffer(), m_VertexBuffer->GetBuffer(), bufferSize);
    }

    void Mesh::CreateIndexBuffer()
    {
        m_HasIndexBuffer = static_cast<uint32_t>(m_Indices.size()) > 0;

        if (!m_HasIndexBuffer) return;

        VkDeviceSize bufferSize = sizeof(m_Indices[0]) * static_cast<uint32_t>(m_Indices.size());
        uint32_t indexSize = sizeof(m_Indices[0]);

        Buffer stagingBuffer{
            m_Device, bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer((void*)m_Indices.data());

        m_IndexBuffer = new Buffer(
            m_Device,bufferSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        m_Device.CopyBuffer(stagingBuffer.GetBuffer(), m_IndexBuffer->GetBuffer(), bufferSize);
    }


    // Helpers
    //--------------------
    void Mesh::LoadObj(const std::string& path)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) 
        {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        for (const auto& shape : shapes) 
        {
            for (const auto& index : shape.mesh.indices) 
            {
                Vertex vertex{};

                vertex.pos =
                {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.uv = 
                {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = { 1.0f, 1.0f, 1.0f };

                if (uniqueVertices.count(vertex) == 0) 
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(m_Vertices.size());
                    m_Vertices.push_back(vertex);
                }

                m_Indices.push_back(uniqueVertices[vertex]);
            }
        }
    }
}
