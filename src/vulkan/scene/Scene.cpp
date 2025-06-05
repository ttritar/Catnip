#include "Scene.h"

namespace cat
{
	// CTOR & DTOR
	//--------------------
	Scene::Scene(Device& device, UniformBuffer<MatrixUbo>* ubo)
		: m_Device{ device }, m_pUniformBuffer(ubo), m_DirectionalLight({})
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
		Model* model = new Model(m_Device,m_pUniformBuffer,path);
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

	void Scene::AddPointLight(const PointLight& light)
	{
		m_PointLights.push_back(light);
	}

	void Scene::RemovePointLight(const PointLight& light)
	{
		auto it = std::remove_if(m_PointLights.begin(), m_PointLights.end(),
			[&](const PointLight& l) { return l.position == light.position; });
		if (it != m_PointLights.end())
		{
			m_PointLights.erase(it, m_PointLights.end());
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
