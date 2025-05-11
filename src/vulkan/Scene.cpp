#include "Scene.h"

namespace cat
{
	// CTOR & DTOR
	//--------------------
	Scene::Scene(Device& device, SwapChain& swapchain, Pipeline* pipeline)
		: m_Device{ device }, m_SwapChain{ swapchain }, m_pGraphicsPipeline{ pipeline }
	{
	}

	Scene::~Scene()
	{
		for (Model* model : m_pModels)
		{
			delete model;
			model = nullptr;
		} 
	}


	// Methods
	//--------------------
	void Scene::Update(float deltaTime)
	{
		// Update logic for the scene can be added here
	}

	Model* Scene::AddModel(const std::string& path)
	{
		Model* model = new Model(m_Device, m_SwapChain, path);
		m_pModels.push_back(model);
		return model;
	}

	void Scene::RemoveModel(const std::string& path)
	{
		auto it = std::remove_if(m_pModels.begin(), m_pModels.end(),
			[&](Model* model) { return model->GetPath() == path; });
		if (it != m_pModels.end())
		{
			delete* it;
			m_pModels.erase(it, m_pModels.end());
		}
	}

	void Scene::Draw(VkCommandBuffer commandBuffer) const
	{
		for (const auto& model : m_pModels)
		{
			vkCmdPushConstants(commandBuffer, m_pGraphicsPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), model->GetTransform());
			model->Draw(commandBuffer);
		}
	}

}
