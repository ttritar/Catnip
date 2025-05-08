#include "Scene.h"

namespace cat
{
	// CTOR & DTOR
	//--------------------
	Scene::Scene(std::vector<Model*> models)
		: m_pModels{ models }
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
	void Scene::AddModel()
	{
		m_pModels.emplace_back();
	}

	void Scene::RemoveModel(const Model* model)
	{
		auto it = std::find(m_pModels.begin(), m_pModels.end(), model);
		if (it != m_pModels.end())
		{
			delete* it; // if Scene owns the Model*
			m_pModels.erase(it);
		}
	}

	void Scene::Draw(VkCommandBuffer commandBuffer) const
	{
		for (const auto& model : m_pModels)
		{
			model->Draw(commandBuffer);
		}
	}

}
