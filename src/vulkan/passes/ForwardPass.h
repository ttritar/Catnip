#pragma once

#include "LightingPass.h"

#include "../Descriptors.h"
#include "../Pipeline.h"

namespace cat
{

	class ForwardPass final
	{
	public:
		// CTOR & DTOR
		//----------------
		ForwardPass(Device& device, SwapChain& swapchain, uint32_t framesInFlight, LightingPass& lightingPass);
		~ForwardPass();

		ForwardPass(const ForwardPass&) = delete;
		ForwardPass(ForwardPass&&) = delete;
		ForwardPass& operator=(const ForwardPass&) = delete;
		ForwardPass& operator=(ForwardPass&&) = delete;

		// METHODS
		//-----------------
		void Record(VkCommandBuffer commandBuffer, uint32_t imageIndex, const Scene& scene, Camera camera) const;
		void Resize(VkExtent2D size);



	private:
		// PRIVATE METHODS
		//-----------------
		void CreateBuffers();
		void CreatePipeline();
		void CreateDescriptors();


		// PRIVATE MEMBERS
		//-----------------
		Device& m_Device;
		const uint32_t m_FramesInFlight;
		SwapChain& m_SwapChain;
		VkExtent2D m_Extent;

		LightingPass& m_LightingPass;

		std::string m_VertPath = "shaders/forward.vert.spv";
		std::string m_FragPath = "shaders/forward.frag.spv";
		Pipeline* m_pPipeline;

		std::unique_ptr<UniformBuffer<MatrixUbo>> m_pUniformBuffer;

		DescriptorSetLayout* m_pUboDescriptorSetLayout;
		DescriptorSetLayout* m_pDescriptorSetLayout;
		DescriptorPool* m_pDescriptorPool;
		DescriptorSet* m_pUboDescriptorSet;
		DescriptorSet* m_pDescriptorSet;
	};
}
