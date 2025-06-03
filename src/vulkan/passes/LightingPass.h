#pragma once
#include "../scene/Scene.h"
#include "../scene/Camera.h"

#include "GeometryPass.h"

namespace cat
{
	class LightingPass final
	{
	public:
		// CTOR & DTOR
		//----------------
		LightingPass(Device& device, VkExtent2D extent, uint32_t framesInFlight,const GeometryPass& geometryPass);
		~LightingPass();

		LightingPass(const LightingPass&) = delete;
		LightingPass(LightingPass&&) = delete;
		LightingPass& operator=(const LightingPass&) = delete;
		LightingPass& operator=(LightingPass&&) = delete;

		// METHODS
		//-----------------
		void Record(VkCommandBuffer commandBuffer, uint32_t imageIndex,
		            Camera camera, Scene& scene) const;
		// Getters & Setters
		const std::vector<std::unique_ptr<Image>>& GetLitImages() const { return m_pLitImages; }

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
		const GeometryPass& m_GeometryPass;


		struct LightingUbo
		{
			alignas(16) glm::vec3 lightDirection;
			alignas(16) glm::vec3 lightColor = { 1.f, 1.f, 1.f };
			alignas(4) float lightIntensity;

			alignas(16) glm::vec3 cameraPosition;
		};
		std::unique_ptr<UniformBuffer<LightingUbo>> m_pUniformBuffer;

		DescriptorPool* m_pDescriptorPool;
		DescriptorSetLayout* m_pUboDescriptorSetLayout;
		DescriptorSetLayout* m_pSamplersDescriptorSetLayout;
		DescriptorSet* m_pUboDescriptorSet;
		DescriptorSet* m_pSamplersDescriptorSet;

		std::string m_VertPath = "shaders/triangle.vert.spv";
		std::string m_FragPath = "shaders/lighting.frag.spv";
		Pipeline* m_pPipeline;

		std::vector<std::unique_ptr<Image>> m_pLitImages;

	};
}
