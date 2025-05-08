#pragma once

#include "Model.h"

#include <vector>

namespace cat
{
	class Scene final
	{
	public:
		// CTOR & DTOR
		//--------------------
		Scene()=default;
		Scene(std::vector<Model*> models);
		~Scene();
		Scene(const Scene&) = delete;
		Scene& operator=(const Scene&) = delete;
		Scene(Scene&&) = delete;
		Scene& operator=(Scene&&) = delete;

		// Methods
		//--------------------
		void AddModel();
		void RemoveModel(const Model* model);
		void Draw(VkCommandBuffer commandBuffer) const;

		// Getters & Setters
		const std::vector<Model*>& GetModels() const { return m_pModels; }

	private:
		// Private members
		//--------------------
		std::vector<Model*> m_pModels;
	};
}
