#pragma once

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "Device.h"

// std
#include <array>
#include "Buffer.h"

namespace cat
{
	class Mesh final
	{
	public:

        struct Vertex
        {
            glm::vec3 pos;
            glm::vec3 color;
            glm::vec2 uv;
            glm::vec3 normal{};


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
        };

		// CTOR & DTOR
		//--------------------
        Mesh(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices);
		Mesh(Device& device, const std::string& path);
		~Mesh();

		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;
        Mesh(Mesh&&) = delete;
		Mesh& operator=(Mesh&&) = delete;

		// Methods
		//--------------------
		void Draw(VkCommandBuffer commandBuffer);
        void Bind(VkCommandBuffer commandBuffer);

	private:
		// Private methods
		//--------------------
        void CreateVertexBuffer();
		void CreateIndexBuffer();
		void LoadObj(const std::string& path);

		// Private Datamembers
		//--------------------
		std::vector<Vertex> m_Vertices;
        Buffer* m_VertexBuffer;

		bool m_HasIndexBuffer = true;
		std::vector<uint16_t> m_Indices;
        Buffer* m_IndexBuffer;

		Device& m_Device;
	};
}
