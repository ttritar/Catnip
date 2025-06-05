#pragma once
#include "../vulkan/scene/Camera.h"
#include "../vulkan/Descriptors.h"
#include "../vulkan/Pipeline.h"
#include "../vulkan/buffers/CommandBuffer.h"
#include "../vulkan/scene/Scene.h"

#include "../vulkan/passes/DepthPrepass.h"
#include "../vulkan/passes/GeometryPass.h"
#include "../vulkan/passes/LightingPass.h"
#include "../vulkan/passes/BlitPass.h"

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
		void RecordPasses() const;
		void ResizePasses() const;

		// Private Members
		//--------------------
		Window& m_Window;
		Camera m_Camera;
		Device m_Device;
		SwapChain* m_pSwapChain;
		Pipeline* m_pGraphicsPipeline;
		Scene* m_pCurrentScene;
		std::vector<Scene*> m_pScenes;
		UniformBuffer<MatrixUbo>* m_pUniformBuffer;
		CommandBuffer* m_pCommandBuffer;

		mutable uint16_t m_CurrentFrame = 0;

		// passes
		std::unique_ptr<DepthPrepass> m_pDepthPrepass;
		std::unique_ptr<GeometryPass> m_pGeometryPass;
		std::unique_ptr<LightingPass> m_pLightingPass;
		std::unique_ptr<BlitPass> m_pBlitPass;

		HDRImage* m_pHDRImage;
	};
}
