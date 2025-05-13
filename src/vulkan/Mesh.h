#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "Device.h"
#include "Buffer.h"
#include "Descriptors.h"

// std
#include <array>
#include <memory>

namespace cat
{
	class Mesh final
	{
	public:
        struct Material {
            std::string diffusePath;
        };

        struct Vertex
        {
            glm::vec3 pos;
            glm::vec3 color;
            glm::vec2 uv;
            //glm::vec3 normal{};


            static VkVertexInputBindingDescription getBindingDescription()
            {
                VkVertexInputBindingDescription bindingDescription{};
                bindingDescription.binding = 0;
                bindingDescription.stride = sizeof(Vertex);
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

                return bindingDescription;
            }

            static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
            {
                std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
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

                return attributeDescriptions;
            }

            bool operator==(const Vertex& other) const
        	{
                return pos == other.pos && color == other.color && uv == other.uv ;
            }
        };

		// CTOR & DTOR
		//--------------------
        Mesh(Device& device, SwapChain& swapchain, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const Material& textures);
		~Mesh();

		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&&) = delete;
		Mesh& operator=(Mesh&&) = delete;

		// Methods
		//--------------------
		void Draw(VkCommandBuffer commandBuffer);
        void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet);

        // Getters & Setters
        VkBuffer GetVertexBuffer()const { return m_VertexBuffer->GetBuffer(); }
        VkBuffer GetIndexBuffer()const { return m_IndexBuffer->GetBuffer(); }

        std::vector<Vertex> GetVertices()const { return m_Vertices; }
        std::vector<uint32_t>GetIndices()const { return  m_Indices; }

		std::vector<Image*> GetImages()const{ return m_Images; }


	private:
		// Private methods
		//--------------------
        void CreateVertexBuffer();
		void CreateIndexBuffer();
		void LoadObj(const std::string& path);

		// Private Datamembers
		//--------------------
        Device& m_Device;

		std::vector<Vertex> m_Vertices;
		uint32_t m_VertexCount = 0;
        Buffer* m_VertexBuffer;
		uint32_t m_VertexBufferSize = 0;

		bool m_HasIndexBuffer = true;
		std::vector<uint32_t> m_Indices;
		uint32_t m_IndexCount = 0;
        Buffer* m_IndexBuffer;
        uint32_t m_IndexBufferSize = 0;

		std::vector<Image*> m_Images;

	};
}
