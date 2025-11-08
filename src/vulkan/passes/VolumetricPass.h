#pragma once

#include "LightingPass.h"
#include "../Descriptors.h"
#include "../Pipeline.h"

namespace cat
{

	class VolumetricPass final
	{
	public:
		// CTOR & DTOR
		//----------------
		VolumetricPass(Device& device, SwapChain& swapChain, uint32_t framesInFlight, LightingPass& lightingPass);
		~VolumetricPass();

		VolumetricPass(const VolumetricPass&) = delete;
		VolumetricPass(VolumetricPass&&) = delete;
		VolumetricPass& operator=(const VolumetricPass&) = delete;
		VolumetricPass& operator=(VolumetricPass&&) = delete;

		// METHODS
		//-----------------
		void Record(VkCommandBuffer commandBuffer, uint32_t imageIndex, const Camera& camera) const;
		void Resize(VkExtent2D size);



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

		std::string m_VertPath = "shaders/triangle.vert.spv";
		std::string m_FragPath = "shaders/volumetric.frag.spv";
		Pipeline* m_pPipeline;

		DescriptorSetLayout* m_pDescriptorSetLayout;
		DescriptorPool* m_pDescriptorPool;
		DescriptorSet* m_pDescriptorSet;
		LightingPass& m_LightingPass;
	};
}
