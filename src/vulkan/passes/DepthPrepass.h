#pragma once
#include "../Pipeline.h"

#include "../scene/Camera.h"
#include "../scene/Scene.h"

namespace cat
{
	class DepthPrepass
	{
	public:
		// CTOR & DTOR
		//------------------------------
		DepthPrepass(Device& device, uint32_t framesInFlight);
		~DepthPrepass();

		DepthPrepass(const DepthPrepass&) = delete;
		DepthPrepass& operator=(const DepthPrepass&) = delete;
		DepthPrepass(DepthPrepass&&) = delete;
		DepthPrepass& operator=(DepthPrepass&&) = delete;


		// METHODS
		//------------------------------
		void Record(VkCommandBuffer commandBuffer, uint32_t imageIndex, Image& depthImage, Camera camera, Scene& scene) const;

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

		std::unique_ptr<UniformBuffer> m_UniformBuffer;
		DescriptorPool* m_DescriptorPool;
		DescriptorSetLayout* m_DescriptorSetLayout;
		DescriptorSet* m_DescriptorSet;

		std::string m_VertPath = "shaders/depth.vert.spv";
		std::string m_FragPath = "";

		Pipeline* m_Pipeline;

	};
}
