#include "Scene.h"

namespace cat
{
	// CTOR & DTOR
	//--------------------
	Scene::Scene(Device& device, SwapChain& swapchain, Pipeline* pipeline, 
		DescriptorSetLayout& layout, DescriptorPool& pool,
		Camera& camera)
		:	m_Device{ device }, m_SwapChain{ swapchain }, m_pGraphicsPipeline{ pipeline },
			m_pDescriptorSetLayout(layout), m_pDescriptorPool(pool),
			m_Camera(camera)
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
	void Scene::Update(float deltaTime, uint16_t currentFrame)
	{
		// Update the uniform buffer for each model
		for (Model* model : m_pModels)
		{
			model->UpdateUniformBuffer(currentFrame, m_Camera.GetView(), m_Camera.GetProjection());
		}

		//...
	}

	Model* Scene::AddModel(const std::string& path)
	{
		Model* model = new Model(m_Device, m_SwapChain, path, m_pDescriptorSetLayout,m_pDescriptorPool);
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

	void Scene::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet) const
	{
		for (const auto& model : m_pModels)
		{
			vkCmdPushConstants(commandBuffer, m_pGraphicsPipeline->GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), model->GetTransform());
			model->Draw(commandBuffer, pipelineLayout, descriptorSet);
		}
	}

}
