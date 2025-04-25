#pragma once

#include "SwapChain.h"

namespace cat
{
	class CommandBuffer final
	{
	public:
		// CTOR & DTOR
		//--------------------
		CommandBuffer(Device& device);
		~CommandBuffer();

		CommandBuffer(const CommandBuffer&) = delete;
		CommandBuffer& operator=(const CommandBuffer&) = delete;
		CommandBuffer(CommandBuffer&&) = delete;
		CommandBuffer& operator=(CommandBuffer&&) = delete;

		// Methods
		//--------------------

		// Getters & Setters
		VkCommandBuffer* GetCommandBuffer(uint16_t idx) { return &m_CommandBuffers[idx]; }
	private:
		// Private Methods
		//--------------------
		void CreateCommandBuffer();

		// Private Members
		//--------------------
		Device& m_Device;
		std::vector<VkCommandBuffer> m_CommandBuffers;
	};
}
