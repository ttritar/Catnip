#pragma once
#include "../scene/Scene.h"
#include "../scene/Camera.h"

namespace cat
{
	class LightingPass final
	{
	public:
		// CTOR & DTOR
		//----------------
		LightingPass(Device& device, VkExtent2D extent, uint32_t framesInFlight);
		~LightingPass();

		LightingPass(const LightingPass&) = delete;
		LightingPass(LightingPass&&) = delete;
		LightingPass& operator=(const LightingPass&) = delete;
		LightingPass& operator=(LightingPass&&) = delete;

		// METHODS
		//-----------------
		void Record(VkCommandBuffer commandBuffer, uint32_t imageIndex,
			Image& depthImage,
			Camera camera, Scene& scene) const;

	private:
		// PRIVATE METHODS
		//-----------------
		void CreateUniformBuffers();
		void CreateDescriptors();
		void CreatePipeline();

		// PRIVATE MEMBERS
		//-----------------
		Device& m_Device;
		uint32_t m_FramesInFlight;
		VkExtent2D m_Extent;

		std::unique_ptr<UniformBuffer> m_pUniformBuffer;

		DescriptorPool* m_pDescriptorPool;
		DescriptorSetLayout* m_pUboDescriptorSetLayout;
		DescriptorSetLayout* m_pSamplersDescriptorSetLayout;
		DescriptorSet* m_pDescriptorSet;

		std::string m_VertPath = "shaders/lighting.vert.spv";
		std::string m_FragPath = "shaders/lighting.frag.spv";
		Pipeline* m_pPipeline;

		std::unique_ptr<Image> m_pLitImage;

	};
}
