#pragma once

#include "LightingPass.h"
#include "../Descriptors.h"
#include "../Pipeline.h"

namespace cat
{

	class BlitPass final
	{
	public:
		// CTOR & DTOR
		//----------------
		BlitPass(Device& device, SwapChain& swapChain, uint32_t framesInFlight, LightingPass& lightingPass);
		~BlitPass();

		BlitPass(const BlitPass&) = delete;
		BlitPass(BlitPass&&) = delete;
		BlitPass& operator=(const BlitPass&) = delete;
		BlitPass& operator=(BlitPass&&) = delete;

		// METHODS
		//-----------------
		void Record(VkCommandBuffer commandBuffer, uint32_t imageIndex, const Camera& camera) const;
		void Resize(VkExtent2D size, const LightingPass& lightingPass);



	private:
		// PRIVATE METHODS
		//-----------------
		void CreatePipeline();
		void CreateDescriptors();


		// PRIVATE MEMBERS
		//-----------------
		Device& m_Device;
		const uint32_t m_FramesInFlight;
		SwapChain& m_SwapChain;
		VkExtent2D m_Extent;

		struct alignas(16) ToneMappingUbo{
			float aperture;
			float shutterSpeed;
			float iso;
		};
		std::unique_ptr<UniformBuffer<ToneMappingUbo>> m_pUniformBuffer;

		std::string m_VertPath = "shaders/triangle.vert.spv";
		std::string m_FragPath = "shaders/blit.frag.spv";
		Pipeline* m_pPipeline;

		DescriptorSetLayout* m_pDescriptorSetLayout;
		DescriptorPool* m_pDescriptorPool;
		DescriptorSet* m_pDescriptorSet;
		LightingPass& m_LightingPass;
	};
}
