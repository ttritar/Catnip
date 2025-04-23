#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Device.h"
#include "Utils.h"

namespace cat
{
	class Model final
	{
	public:
		// CTOR & DTOR
		//--------------------
		Model(Device& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);	
		~Model();

		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;
		Model(Model&&) = delete;
		Model& operator=(Model&&) = delete;

		// Methods
		//--------------------
		void Bind(VkCommandBuffer commandBuffer) const;
		void Draw(VkCommandBuffer commandBuffer) const;

		// Getters & Setters

	private:
		// Private Methods
		//--------------------

		// Creators
		void CreateVertexBuffer();
		void CreateIndexBuffer();

		// Helpers
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

		// Private Members
		//--------------------
		const std::vector<Vertex>& m_Vertices;
		VkBuffer m_VertexBuffer;
		VkDeviceMemory m_VertexBufferMemory;

		const std::vector<uint32_t>& m_Indices;
		VkBuffer m_IndexBuffer;
		VkDeviceMemory m_IndexBufferMemory;


		Device& m_Device;

	};
}