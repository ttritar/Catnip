#pragma once

#include "Model.h"
#include "Pipeline.h"
#include "../core/Camera.h"

#include <vector>

namespace cat
{
	class Scene final
	{
	public:
		// CTOR & DTOR
		//--------------------
		Scene(Device& device, SwapChain& swapchain, Pipeline* pipeline, 
			DescriptorSetLayout& layout, DescriptorPool& pool,
			Camera& camera);
		~Scene();

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;
		Scene(Scene&&) = delete;
		Scene& operator=(Scene&&) = delete;

		// Methods
		//--------------------
		void Update(float deltaTime, uint16_t currentFrame);

		Model* AddModel(const std::string& path);
		void RemoveModel(const std::string& path);

		void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet) const;

		// Getters & Setters
		const std::vector<Model*>& GetModels() const { return m_pModels; }
		std::vector<Image*> GetImages() const
		{
			std::vector<Image*> images;
			for (auto& model : m_pModels)
			{
				const auto& modelImages = model->GetImages();
				images.insert(images.end(), modelImages.begin(), modelImages.end());
			}
			return images;
		}

	private:
		// Private members
		//--------------------
		Device& m_Device;
		SwapChain& m_SwapChain;
		Pipeline* m_pGraphicsPipeline;
		DescriptorSetLayout& m_pDescriptorSetLayout;
		DescriptorPool& m_pDescriptorPool;

		Camera m_Camera;
		std::vector<Model*> m_pModels;
	};
}
