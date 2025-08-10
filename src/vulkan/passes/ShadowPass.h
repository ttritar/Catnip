#pragma once
#include "../Pipeline.h"

#include "../scene/Camera.h"
#include "../scene/Scene.h"

namespace cat
{
	class ShadowPass
	{
	public:
		// CTOR & DTOR
		//------------------------------
		ShadowPass(Device& device, uint32_t framesInFlight, SwapChain& swapchain);
		~ShadowPass();

		ShadowPass(const ShadowPass&) = delete;
		ShadowPass& operator=(const ShadowPass&) = delete;
		ShadowPass(ShadowPass&&) = delete;
		ShadowPass& operator=(ShadowPass&&) = delete;


		// METHODS
		//------------------------------
		void Record(VkCommandBuffer commandBuffer, uint32_t imageIndex, Camera camera, Scene& scene) const;

	private:
		// Private methods
		//------------------------------
		void CreateUniformBuffers();
		void CreateDescriptors();
		void CreatePipeline();



		// Private members
		//------------------------------
		Device& m_Device;
		uint32_t m_FramesInFlight;

		std::vector<std::unique_ptr<Image>> m_pDepthImages;

		struct alignas(16) ShadowUbo
		{
			glm::mat4 lightProj;
			glm::mat4 lightView;
		};
		std::unique_ptr<UniformBuffer<ShadowUbo>> m_pUniformBuffer;
		DescriptorPool* m_pDescriptorPool;
		DescriptorSetLayout* m_pDescriptorSetLayout;
		DescriptorSet* m_pDescriptorSet;

		std::string m_VertPath = "shaders/shadow.vert.spv";
		std::string m_FragPath = "";

		Pipeline* m_pPipeline;

	};
}
