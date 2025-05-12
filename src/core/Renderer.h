#pragma once
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
		Device m_Device;
		SwapChain* m_pSwapChain;
		DescriptorSetLayout* m_pDescriptorSetLayout;
		Pipeline* m_pGraphicsPipeline;
		Scene* m_pScene;
		//const std::vector<cat::Mesh::Vertex> m_Vertices = {
		//	{{-0.5f, -0.5f,0.0f},  {1.0f, 0.0f, 0.0f},    {1.0f,0.0f}},
		//	{{0.5f, -0.5f,0.0f},   {0.0f, 1.0f, 0.0f},    {0.0f,0.0}},
		//	{{0.5f, 0.5f,0.0f},    {0.0f, 0.0f, 1.0f},    {0.0f,1.0f,}},
		//	{{-0.5f, 0.5f,0.0f},   {1.0f, 1.0f, 1.0f},    {1.0f,1.0f}},
		//
		//	{{-0.5f, -0.5f, -0.5f},     {1.0f, 0.0f, 0.0f},     {0.0f, 0.0f}},
		//	{{0.5f, -0.5f, -0.5f},      {0.0f, 1.0f, 0.0f},     {1.0f, 0.0f}},
		//	{{0.5f, 0.5f, -0.5f},       {0.0f, 0.0f, 1.0f},     {1.0f, 1.0f}},
		//	{{-0.5f, 0.5f, -0.5f},      {1.0f, 1.0f, 1.0f},     {0.0f, 1.0f}}
		//};
		//const std::vector<uint16_t> m_Indices = {
		//	0, 1, 2, 2, 3, 0,
		//	4, 5, 6, 6, 7, 4
		//};
		UniformBuffer* m_pUniformBuffer;
		DescriptorPool* m_pDescriptorPool;
		DescriptorSet* m_pDescriptorSet;
		CommandBuffer* m_pCommandBuffer;

		mutable uint16_t m_CurrentFrame = 0;

	};
}
