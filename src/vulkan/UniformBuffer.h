#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "SwapChain.h"

namespace cat
{
	class UniformBuffer
	{
	public:
		struct UniformBufferObject
		{
			alignas(16) glm::mat4 model;
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};


		// CTOR & DTOR
		//--------------------
		UniformBuffer(Device& device, SwapChain* swapChain);
		~UniformBuffer();

		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer& operator=(const UniformBuffer&) = delete;
		UniformBuffer(UniformBuffer&&) = delete;
		UniformBuffer& operator=(UniformBuffer&&) = delete;

		// Methods
		//--------------------
		void Update(unsigned int currentImage, const glm::mat4& model ,const glm::mat4& view, const glm::mat4& proj);

		// Getters & Setters
		VkBuffer GetUniformBuffer(uint16_t idx)const { return m_UniformBuffers[idx]; }
		std::vector<VkBuffer> GetUniformBuffers()const { return m_UniformBuffers; }
		VkDeviceMemory UniformBufferMemory(uint16_t idx)const { return m_UniformBuffersMemory[idx]; }
		std::vector<VkDeviceMemory> UniformBuffersMemory()const { return m_UniformBuffersMemory; }
		void* GetUniformBufferMapped(uint16_t idx)const { return m_UniformBuffersMapped[idx]; }
		std::vector<void*> GetUniformBuffersMapped()const { return m_UniformBuffersMapped; }

	private:
		// Private Members
		//--------------------
		Device& m_Device;
		SwapChain* m_SwapChain;

		std::vector<VkBuffer> m_UniformBuffers;
		std::vector<VkDeviceMemory> m_UniformBuffersMemory;
		std::vector<void*> m_UniformBuffersMapped;
	};
}
