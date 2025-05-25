#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Buffer.h"
#include "SwapChain.h"

namespace cat
{
	class UniformBuffer
	{
	public:
		struct UniformBufferObject
		{
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
		void Update(unsigned int currentImage, glm::mat4 view, glm::mat4 proj);

		// Getters & Setters
		VkBuffer GetUniformBuffer(uint16_t idx)const { return m_UniformBuffers[idx]->GetBuffer(); }
		std::vector<VkBuffer> GetUniformBuffers()const
		{
			std::vector<VkBuffer> buffers;
			for (const auto& buffer : m_UniformBuffers)
			{
				buffers.push_back(buffer->GetBuffer());
			}
			return buffers;
		}

	private:
		// Private Members
		//--------------------
		Device& m_Device;
		SwapChain* m_SwapChain;

		std::vector<std::unique_ptr<Buffer>> m_UniformBuffers;
	};
}
