#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Buffer.h"
#include "../SwapChain.h"

namespace cat
{
	struct MatrixUbo
	{
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	template<typename T>
	class UniformBuffer final
	{
	public:
		// CTOR & DTOR
		//--------------------
		UniformBuffer(Device& device, uint32_t count = cat::MAX_FRAMES_IN_FLIGHT)
			: m_Device(device)
		{
			VkDeviceSize bufferSize = sizeof(T);

			m_UniformBuffers.resize(count);
			m_BufferInfos.resize(count);

			for (uint32_t i = 0; i < count; ++i)
			{
				m_UniformBuffers[i] = std::make_unique<Buffer>(
					m_Device,
					Buffer::BufferInfo{ bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU }
				);

				m_BufferInfos[i] = m_UniformBuffers[i]->GetDescriptorBufferInfo();
			}
			
		}
		~UniformBuffer()
		{
			for (auto& buffer : m_UniformBuffers)
			{
				buffer->Unmap();
			}
		}

		UniformBuffer(const UniformBuffer&) = delete;
		UniformBuffer& operator=(const UniformBuffer&) = delete;
		UniformBuffer(UniformBuffer&&) = delete;
		UniformBuffer& operator=(UniformBuffer&&) = delete;

		// Methods
		//--------------------
		void Update(uint32_t frameIndex, T& data)
		{
			m_UniformBuffers[frameIndex]->Map();
			m_UniformBuffers[frameIndex]->WriteToBuffer(&data);
			m_UniformBuffers[frameIndex]->Unmap();
		}

		// Getters & Setters
		VkBuffer GetBuffer(uint16_t idx)const { return m_UniformBuffers[idx]->GetBuffer(); }
		std::vector<VkBuffer> GetBuffers()const
		{
			std::vector<VkBuffer> buffers;
			for (const auto& buffer : m_UniformBuffers)
			{
				buffers.push_back(buffer->GetBuffer());
			}
			return buffers;
		}

		const std::vector<VkDescriptorBufferInfo>& GetDescriptorBufferInfos() const
		{
			return m_BufferInfos;
		}

	private:
		// Private Members
		//--------------------
		Device& m_Device;

		std::vector<std::unique_ptr<Buffer>> m_UniformBuffers;
		std::vector<VkDescriptorBufferInfo> m_BufferInfos;

	};
}
