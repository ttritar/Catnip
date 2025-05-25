#pragma once

#include "Model.h"
#include "../Pipeline.h"

#include <vector>

namespace cat
{
	class Scene final
	{
	public:
		// CTOR & DTOR
		//--------------------
		Scene(Device& device, SwapChain& swapchain, Pipeline* pipeline, UniformBuffer* ubo);
		~Scene();

		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;
		Scene(Scene&&) = delete;
		Scene& operator=(Scene&&) = delete;

		// Methods
		//--------------------
		void Update(float deltaTime);

		Model* AddModel(const std::string& path);
		void RemoveModel(const std::string& path);

		void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t frameIdx, bool isDepthPass = 0) const;

		// Getters & Setters
		const std::vector<Model*>& GetModels() const { return m_pModels; }

	private:
		// Private members
		//--------------------
		Device& m_Device;
		SwapChain& m_SwapChain;
		Pipeline* m_pGraphicsPipeline;
		UniformBuffer* m_pUniformBuffer;
		DescriptorPool* m_pDescriptorPool;
		
		std::vector<Model*> m_pModels;
	};
}
