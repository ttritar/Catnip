#include "UniformBuffer.h"

// std
#include <chrono>


namespace cat
{

	UniformBuffer::UniformBuffer(Device& device, SwapChain* swapChain)
		: m_Device(device), m_SwapChain(swapChain)
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		m_UniformBuffers.resize(cat::MAX_FRAMES_IN_FLIGHT);

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		for (size_t i = 0; i < cat::MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_UniformBuffers[i] = std::make_unique<Buffer>(
				m_Device, bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VMA_MEMORY_USAGE_CPU_TO_GPU, true
				
			);
			m_UniformBuffers[i]->Map();
		}
	}

	UniformBuffer::~UniformBuffer()
	{
		for (size_t i = 0; i < cat::MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_UniformBuffers[i]->Unmap();
		}
	}

	void UniformBuffer::Update(uint32_t currentImage, glm::mat4 view, glm::mat4 proj)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();


		// MODEL ROTATION
		UniformBufferObject ubo{};
		ubo.model = glm::mat4(1.f);

		// VIEW TRANSFORMATION
		ubo.view = view;

		// PERSPECTIVE PROJECTION   
		ubo.proj = proj;


		// Copy data to uniform buffer
		m_UniformBuffers[currentImage]->Map();
		m_UniformBuffers[currentImage]->WriteToBuffer(&ubo, sizeof(UniformBufferObject));
		m_UniformBuffers[currentImage]->Unmap();
	}
}
