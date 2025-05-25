#include "CommandBuffer.h"

#include <stdexcept>

namespace cat
{
	CommandBuffer::CommandBuffer(Device& device)
		: m_Device(device)
	{
		CreateCommandBuffer();
	}
	CommandBuffer::~CommandBuffer()
	{
		vkFreeCommandBuffers(m_Device.GetDevice(), m_Device.GetCommandPool(), static_cast<uint32_t>(m_CommandBuffers.size()), m_CommandBuffers.data());
	}

	void CommandBuffer::CreateCommandBuffer()
	{
		m_CommandBuffers.resize(cat::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_Device.GetCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // is secondary or primary cmd buffer
		allocInfo.commandBufferCount = (uint32_t)m_CommandBuffers.size();

		if (vkAllocateCommandBuffers(m_Device.GetDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}
}
