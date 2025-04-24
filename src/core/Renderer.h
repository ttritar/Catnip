#pragma once
#include "../vulkan/SwapChain.h"

namespace cat
{
	class Pipeline;
	class Model;
	class Window;

	class Renderer final
	{
	public:
		// CTOR & DTOR
		//--------------------
		Renderer(Window& window, Device& device);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		// Methods
		//--------------------
		void Render()const;

		// Getters & Setters


	private:
		// Private Methods
		//--------------------
		void InitializeVulkan();

		// Creators
		void CreateCommandBuffer();

		// Helpers


		// Private Members
		//--------------------
		Window& m_Window;
		Device& m_Device;
		SwapChain* m_pSwapChain;
	};
}
