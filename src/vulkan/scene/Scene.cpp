#include "Scene.h"

namespace cat
{
	// CTOR & DTOR
	//--------------------
	Scene::Scene(Device& device, SwapChain& swapchain, UniformBuffer* ubo)
		: m_Device{ device }, m_SwapChain{ swapchain }, m_pUniformBuffer(ubo)
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
		Model* model = new Model(m_Device, m_SwapChain,m_pUniformBuffer,path);
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

	void Scene::AddLight(const Light& light)
	{
		m_Lights.push_back(light);
	}

	void Scene::RemoveLight(const Light& light)
	{
		auto it = std::remove_if(m_Lights.begin(), m_Lights.end(),
			[&](const Light& l) { return l.position == light.position && l.color == light.color && l.intensity == light.intensity; });
		if (it != m_Lights.end())
		{
			m_Lights.erase(it, m_Lights.end());
		}
	}

	void Scene::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t frameIdx, bool isDepthPass) const
	{
		for (const auto& model : m_pModels)
		{
			vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), model->GetTransform());
			model->Draw(commandBuffer, pipelineLayout, frameIdx, isDepthPass);
		}
	}

}
