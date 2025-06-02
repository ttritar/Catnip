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
		BlitPass(Device& device, VkExtent2D extent, uint32_t framesInFlight, const LightingPass& lightingPass);
		~BlitPass();

		BlitPass(const BlitPass&) = delete;
		BlitPass(BlitPass&&) = delete;
		BlitPass& operator=(const BlitPass&) = delete;
		BlitPass& operator=(BlitPass&&) = delete;

		// METHODS
		//-----------------
		void Record(VkCommandBuffer commandBuffer, uint32_t imageIndex,
		            Image& swapchainImage) const;



	private:
		// PRIVATE METHODS
		//-----------------
		void CreatePipeline();
		void CreateDescriptors();


		// PRIVATE MEMBERS
		//-----------------
		Device& m_Device;
		uint32_t m_FramesInFlight;
		VkExtent2D m_Extent;

		std::string m_VertPath = "shaders/triangle.vert.spv";
		std::string m_FragPath = "shaders/blit.frag.spv";
		Pipeline* m_pPipeline;

		DescriptorSetLayout* m_pDescriptorSetLayout;
		DescriptorPool* m_pDescriptorPool;
		DescriptorSet* m_pDescriptorSet;
		Image& m_LitImage;
	};
}
