#pragma once
#include "Device.h"

namespace cat
{
	class CommandBuffer final
	{
	public:
		CommandBuffer(Device& device);
		~CommandBuffer();

		CommandBuffer(const CommandBuffer&) = delete;
		CommandBuffer& operator=(const CommandBuffer&) = delete;
		CommandBuffer(CommandBuffer&&) = delete;
		CommandBuffer& operator=(CommandBuffer&&) = delete;
	private:

	};
}
