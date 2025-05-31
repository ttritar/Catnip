#include "UniformBuffer.h"

// std
#include <chrono>


namespace cat
{

	UniformBuffer::UniformBuffer(Device& device, uint32_t count)
		: m_Device(device)
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);

		m_UniformBuffers.resize(count);
		m_BufferInfos.resize(count);

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		for (size_t i = 0; i < count; i++)
		{
			m_UniformBuffers[i] = std::make_unique<Buffer>(m_Device, 
				Buffer::BufferInfo{ bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,VMA_MEMORY_USAGE_CPU_TO_GPU }
			);

			m_BufferInfos[i] = m_UniformBuffers[i]->GetDescriptorBufferInfo();
		}
	}

	UniformBuffer::~UniformBuffer()
	{
		for (size_t i = 0; i < m_UniformBuffers.size(); i++)
		{
			m_UniformBuffers[i]->Unmap();
		}
	}

	void UniformBuffer::Update(uint32_t currentImage, glm::mat4 view, glm::mat4 proj)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();


		UniformBufferObject ubo{};

		// VIEW TRANSFORMATION
		ubo.view = view;

		// PERSPECTIVE PROJECTION   
		ubo.proj = proj;


		// Copy data to uniform buffer
		m_UniformBuffers[currentImage]->Map();
		m_UniformBuffers[currentImage]->WriteToBuffer(&ubo);
		m_UniformBuffers[currentImage]->Unmap();
	}
}
