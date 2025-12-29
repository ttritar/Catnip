#pragma once

#include <iostream>

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
		VolumetricPass(Device& device, SwapChain& swapChain, uint32_t framesInFlight, LightingPass& lightingPass, ShadowPass& shadowPass);
		~VolumetricPass();

		VolumetricPass(const VolumetricPass&) = delete;
		VolumetricPass(VolumetricPass&&) = delete;
		VolumetricPass& operator=(const VolumetricPass&) = delete;
		VolumetricPass& operator=(VolumetricPass&&) = delete;

		// METHODS
		//-----------------
		void Record(VkCommandBuffer commandBuffer, uint32_t frameIndex, Camera camera, Scene& scene) const;
		void Resize(VkExtent2D size);

		// Getters & Setters
		const std::vector<std::unique_ptr<Image>>& GetVolumetricImages() const { return m_pVolumetricImages; }
		void ToggleUseMultiScattering()
		{
			m_UseMultiScattering = !m_UseMultiScattering;
			std::cout << "Using multiscattering: " << m_UseMultiScattering << std::endl;
		}

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

		std::string m_VertPath = "shaders/triangle.vert.spv";
		std::string m_FragPath = "shaders/volumetric.frag.spv";
		Pipeline* m_pPipeline;

		DescriptorSetLayout* m_pDescriptorSetLayout;
		DescriptorPool* m_pDescriptorPool;
		DescriptorSet* m_pDescriptorSet;
		ShadowPass& m_ShadowPass;
		LightingPass& m_LightingPass;

		bool m_UseMultiScattering = false;
		struct alignas(16) VolumetricsUbo
		{
			glm::mat4 invViewProj;
			glm::mat4 lightViewProj;

			glm::vec3 camPos;
			float _padding1;

			glm::vec3 lightDir;
			float _padding2;
			glm::vec3 lightColor;
			float lightIntensity;
			float volumetricDensity;

			float stepSize;
			int numSteps;

			float rayStrength;
			float rayDecay;
			float rayDensity;
			float rayWeight;

			int useMultiScattering = 0;
			float multiScatterStrength;
			float _padding3[3];
		};
		std::unique_ptr<UniformBuffer<VolumetricsUbo>> m_pUniformBuffer;

		std::vector<std::unique_ptr<Image>> m_pVolumetricImages;
	};
}
