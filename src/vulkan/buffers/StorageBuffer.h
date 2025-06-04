#pragma once

#include "Buffer.h"

namespace cat
{
	template<typename T, std::size_t capacityCount >
	class StorageBuffer final
	{
	public:
		// CTOR & DTOR
		//--------------------
		StorageBuffer(Device& device ,uint32_t count = cat::MAX_FRAMES_IN_FLIGHT)
			: m_Device(device)
		{
			static constexpr VkDeviceSize bufferSize = sizeof(T) * capacityCount;

			m_StorageBuffers.resize(count);
			m_BufferInfos.resize(count);

			for (uint32_t i = 0; i < count; ++i)
			{
				m_StorageBuffers[i] = std::make_unique<Buffer>(
					m_Device,
					Buffer::BufferInfo{ bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU }
				);

				m_BufferInfos[i] = m_StorageBuffers[i]->GetDescriptorBufferInfo();
			}

		}
		~StorageBuffer()
		{
			for (auto& buffer : m_StorageBuffers)
			{
				buffer->Unmap();
			}
		}

		StorageBuffer(const StorageBuffer&) = delete;
		StorageBuffer& operator=(const StorageBuffer&) = delete;
		StorageBuffer(StorageBuffer&&) = delete;
		StorageBuffer& operator=(StorageBuffer&&) = delete;

		// Methods
		//--------------------
		void Update(uint32_t frameIndex, std::array<T,capacityCount>& data)
		{
			m_StorageBuffers[frameIndex]->Map();
			m_StorageBuffers[frameIndex]->WriteToBuffer(data.data());
			m_StorageBuffers[frameIndex]->Unmap();
		}

		// Getters & Setters
		VkBuffer GetBuffer(uint16_t idx)const { return m_StorageBuffers[idx]->GetBuffer(); }
		std::vector<VkBuffer> GetBuffers()const
		{
			std::vector<VkBuffer> buffers;
			for (const auto& buffer : m_StorageBuffers)
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

		std::vector<std::unique_ptr<Buffer>> m_StorageBuffers;
		std::vector<VkDescriptorBufferInfo> m_BufferInfos;

	};
}
