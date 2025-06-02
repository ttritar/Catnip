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
		Image& GetAlbedoBuffer() const { return *m_pAlbedoBuffer; }
		Image& GetNormalBuffer() const { return *m_pNormalBuffer; }
		Image& GetSpecularBuffer() const { return *m_pSpecularBuffer; }
		Image& GetWorldBuffer() const { return *m_pWorldBuffer; }


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

		std::unique_ptr<Image> m_pAlbedoBuffer;
		std::unique_ptr<Image> m_pNormalBuffer;
		std::unique_ptr<Image> m_pSpecularBuffer;
		std::unique_ptr<Image> m_pWorldBuffer;
	};

}
