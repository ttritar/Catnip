#pragma once
#include "../scene/Scene.h"
#include "../scene/Camera.h"

namespace cat
{
	class GeometryPass
	{
	public:
		// CTOR & DTOR
		//----------------
		GeometryPass(Device& device, VkExtent2D extent, uint32_t framesInFlight);
		~GeometryPass();

		GeometryPass(const GeometryPass&) = delete;
		GeometryPass(GeometryPass&&) = delete;
		GeometryPass& operator=(const GeometryPass&) = delete;
		GeometryPass& operator=(GeometryPass&&) = delete;

		//METHODS
		//-----------------
		void Record(VkCommandBuffer commandBuffer, uint32_t imageIndex,
			Image& depthImage,
		            Camera camera, Scene& scene) const;

		// Getters & Setters
		Image& GetAlbedoBuffer(int idx) const { return *m_pAlbedoBuffers[idx]; }
		Image& GetNormalBuffer(int idx) const { return *m_pNormalBuffers[idx]; }
		Image& GetSpecularBuffer(int idx) const { return *m_pSpecularBuffers[idx]; }
		Image& GetWorldBuffer(int idx) const { return *m_pWorldBuffers[idx]; }


	private:
		// PRIVATE METHODS
		//-----------------
		void CreateUniformBuffers();
		void CreateDescriptors();
		void CreatePipeline();

		//PRIVATE MEMBERS
		//-----------------
		Device& m_Device;
		uint32_t m_FramesInFlight;
		VkExtent2D m_Extent;

		
		std::unique_ptr<UniformBuffer<MatrixUbo>> m_pUniformBuffer;

		DescriptorPool* m_pDescriptorPool;
		DescriptorSetLayout* m_pUboDescriptorSetLayout;
		DescriptorSetLayout* m_pSamplersDescriptorSetLayout;
		DescriptorSet* m_pDescriptorSet;

		std::string m_VertPath = "shaders/geometry.vert.spv";
		std::string m_FragPath = "shaders/geometry.frag.spv";

		Pipeline* m_pPipeline;

		std::vector<std::unique_ptr<Image>> m_pAlbedoBuffers;
		std::vector<std::unique_ptr<Image>> m_pNormalBuffers;
		std::vector<std::unique_ptr<Image>> m_pSpecularBuffers;
		std::vector<std::unique_ptr<Image>> m_pWorldBuffers;
	};

}
