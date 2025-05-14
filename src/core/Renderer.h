#pragma once
#include "Camera.h"
#include "../vulkan/Descriptors.h"
#include "../vulkan/Pipeline.h"
#include "../vulkan/UniformBuffer.h"
#include "../vulkan/CommandBuffer.h"
#include "../vulkan/Scene.h"

namespace cat
{
	class Window;

	class Renderer final
	{
	public:
		// CTOR & DTOR
		//--------------------
		Renderer(Window& window);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		// Methods
		//--------------------
		void Update(float deltaTime);
		void Render()const;

		// Getters & Setters


	private:
		// Private Methods
		//--------------------
		void InitializeVulkan();
		void DrawFrame()const;
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) const;

		// Private Members
		//--------------------
		Window& m_Window;
		Camera m_Camera;
		Device m_Device;
		SwapChain* m_pSwapChain;
		Pipeline* m_pGraphicsPipeline;
		Scene* m_pScene;
		UniformBuffer* m_pUniformBuffer;
		CommandBuffer* m_pCommandBuffer;

		mutable uint16_t m_CurrentFrame = 0;

	};
}
