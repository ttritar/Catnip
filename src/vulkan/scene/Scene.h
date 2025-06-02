#pragma once

#include "Model.h"
#include "../Pipeline.h"

#include <vector>

namespace cat
{
	class Scene final
	{
	public:
		struct Light
		{
			glm::vec3 direction;
			glm::vec3 color = {0.f,0.f,0.f};
			float intensity = 1.f;

		};

		// CTOR & DTOR
		//--------------------
		Scene(Device& device, SwapChain& swapchain, UniformBuffer<MatrixUbo>* ubo);
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

		void AddLight(const Light& light);
		void RemoveLight(const Light& light);

		void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t frameIdx, bool isDepthPass = 0) const;

		// Getters & Setters
		const std::vector<Model*>& GetModels() const { return m_pModels; }
		const std::vector<Light>& GetLights() const { return m_Lights; }

	private:
		// Private members
		//--------------------
		Device& m_Device;
		SwapChain& m_SwapChain;
		UniformBuffer<MatrixUbo>* m_pUniformBuffer;
		
		std::vector<Model*> m_pModels;
		std::vector<Light> m_Lights;
	};
}
