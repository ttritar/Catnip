#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "../Device.h"
#include "../buffers/Buffer.h"
#include "../Descriptors.h"

// std
#include <array>
#include <memory>

namespace cat
{
    class Mesh final
    {
    public:

        struct Material
        {
            std::string albedoPath;
            std::string normalPath;
            std::string specularPath;

            const int amount = 3;
        };

        struct Vertex
        {
            glm::vec3 pos;
            glm::vec3 color;
            glm::vec2 uv;
            glm::vec3 normal;
            glm::vec3 tangent;
            glm::vec3 bitangent;


            static VkVertexInputBindingDescription getBindingDescription()
            {
                VkVertexInputBindingDescription bindingDescription{};
                bindingDescription.binding = 0;
                bindingDescription.stride = sizeof(Vertex);
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

                return bindingDescription;
            }

            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions()
            {
                std::vector<VkVertexInputAttributeDescription> attributeDescriptions(6);

                attributeDescriptions[0].binding = 0;   //from which binding the per-vertex data comes
                attributeDescriptions[0].location = 0;  //location directive in the vertex shader 
                attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;  //vec2
                attributeDescriptions[0].offset = offsetof(Vertex, pos);  //where this data starts in the struct   

                attributeDescriptions[1].binding = 0;
                attributeDescriptions[1].location = 1;
                attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;  //vec3
                attributeDescriptions[1].offset = offsetof(Vertex, color);

                attributeDescriptions[2].binding = 0;
                attributeDescriptions[2].location = 2;
                attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
                attributeDescriptions[2].offset = offsetof(Vertex, uv);

                attributeDescriptions[3].binding = 0;
                attributeDescriptions[3].location = 3;
                attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[3].offset = offsetof(Vertex, normal);

                attributeDescriptions[4].binding = 0;
                attributeDescriptions[4].location = 4;
                attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[4].offset = offsetof(Vertex, tangent);

                attributeDescriptions[5].binding = 0;
                attributeDescriptions[5].location = 5;
                attributeDescriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[5].offset = offsetof(Vertex, bitangent);

                return attributeDescriptions;
            }

            bool operator==(const Vertex& other) const
            {
                return pos == other.pos && color == other.color && uv == other.uv;
            }
        };

        struct RawMeshData
        {
            std::vector<Vertex> vertices;
            std::vector<uint32_t> indices;
            Material material;
            glm::mat4 transform;
            bool opaque = true;
        };

        // CTOR & DTOR
        //--------------------
        Mesh(Device& device, UniformBuffer<MatrixUbo>* ubo,
            DescriptorSetLayout* layout, DescriptorPool* pool,
            const RawMeshData& meshData);
        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&&) = delete;
        Mesh& operator=(Mesh&&) = delete;

        // Methods
        //--------------------
        void Draw(VkCommandBuffer commandBuffer);
        void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t idx, bool isDepthPass);

        // Getters & Setters
        VkBuffer GetVertexBuffer()const { return m_VertexBuffer->GetBuffer(); }
        VkBuffer GetIndexBuffer()const { return m_IndexBuffer->GetBuffer(); }

        std::vector<Vertex> GetVertices()const { return m_Vertices; }
        std::vector<uint32_t>GetIndices()const { return  m_Indices; }

        const glm::mat4& GetTransform() const { return m_Transform; }


    private:
        // Private methods
        //--------------------
        void CreateVertexBuffer();
        void CreateIndexBuffer();

        // Private Datamembers
        //--------------------
        Device& m_Device;
        DescriptorSet* m_pDescriptorSet;

        std::vector<Vertex> m_Vertices;
        uint32_t m_VertexCount = 0;
        std::unique_ptr<Buffer> m_VertexBuffer;
        uint32_t m_VertexBufferSize = 0;

        bool m_HasIndexBuffer = true;
        std::vector<uint32_t> m_Indices;
        uint32_t m_IndexCount = 0;
        std::unique_ptr<Buffer> m_IndexBuffer;
        uint32_t m_IndexBufferSize = 0;

        std::vector<Image*> m_Images;

        const glm::mat4 m_Transform = glm::mat4(1.0f);

    };
}