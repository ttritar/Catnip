#pragma once
#include "../scene/Scene.h"
#include "../scene/Camera.h"

#include "../buffers/StorageBuffer.h"

#include "GeometryPass.h"

namespace cat
{
	class LightingPass final
	{
	public:
		static constexpr size_t MAX_POINT_LIGHTS = 16;

		// CTOR & DTOR
		//----------------
		LightingPass(Device& device, VkExtent2D extent, uint32_t framesInFlight, const GeometryPass& geometryPass,
		 	HDRImage* pSkyBoxImage, SwapChain& swapchain);
		~LightingPass();

		LightingPass(const LightingPass&) = delete;
		LightingPass(LightingPass&&) = delete;
		LightingPass& operator=(const LightingPass&) = delete;
		LightingPass& operator=(LightingPass&&) = delete;

		// METHODS
		//-----------------
		void Record(VkCommandBuffer commandBuffer, uint32_t imageIndex,
		            Camera camera, Scene& scene) const;
		void Resize(VkExtent2D size, const GeometryPass& geometryPass);

		// Getters & Setters
		const std::vector<std::unique_ptr<Image>>& GetLitImages() const { return m_pLitImages; }

	private:
		// PRIVATE METHODS
		//-----------------
		void CreateBuffers();
		void CreateDescriptors();
		void CreatePipeline();

		// PRIVATE MEMBERS
		//-----------------
		Device& m_Device;
		SwapChain& m_SwapChain;
		uint32_t m_FramesInFlight;
		VkExtent2D m_Extent;
		const GeometryPass& m_GeometryPass;


		struct alignas(16) LightingUbo
		{
			glm::vec3 lightDirection;
			const float padding{};
			glm::vec3 lightColor = { 1.f, 1.f, 1.f };
			float lightIntensity;

			glm::vec4 cameraPosition;
			glm::mat4 proj;
			glm::mat4 view;
			glm::vec2 viewportSize;

			uint32_t pointLightCount;
		};
		std::unique_ptr<UniformBuffer<LightingUbo>> m_pUniformBuffer;
		std::unique_ptr<StorageBuffer<Scene::PointLight, MAX_POINT_LIGHTS>> m_pPointLightingStorageBuffer;

		std::unique_ptr<DescriptorPool> m_pDescriptorPool;
		std::unique_ptr<DescriptorSetLayout> m_pUboDescriptorSetLayout;
		std::unique_ptr<DescriptorSetLayout> m_pSamplersDescriptorSetLayout;
		std::unique_ptr<DescriptorSetLayout> m_pHDRISamplersDescriptorSetLayout;

		std::unique_ptr<DescriptorSet> m_pUboDescriptorSet;
		std::unique_ptr<DescriptorSet> m_pSamplersDescriptorSet;
		std::unique_ptr<DescriptorSet> m_pHDRISamplersDescriptorSet;

		std::string m_VertPath = "shaders/triangle.vert.spv";
		std::string m_FragPath = "shaders/lighting.frag.spv";
		Pipeline* m_pPipeline;

		std::vector<std::unique_ptr<Image>> m_pLitImages;
		HDRImage* m_pSkyBoxImage;

	};
}
