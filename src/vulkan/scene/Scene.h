#pragma once

#include "HDRImage.h"
#include "Model.h"
#include "../Pipeline.h"

#include <vector>

namespace cat
{
	class Scene final
	{
	public:
		struct alignas(16) PointLight
		{
			glm::vec4 position {0.f };

			glm::vec4 color { 1.f };
			float intensity = 1.f;
			float radius = 10.f;
		};

		struct DirectionalLight
		{
			glm::vec3 direction = { 0.f,-1.f,0.f };
			glm::vec3 color {1.0f};
			float intensity = 5.f;
		};

		// CTOR & DTOR
		//--------------------
		Scene(Device& device, UniformBuffer<MatrixUbo>* ubo);
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

		void SetDirectionalLight(const DirectionalLight& light) { m_DirectionalLight = light; }
		void AddPointLight(const PointLight& light);
		void RemovePointLight(const PointLight& light);

		void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t frameIdx, bool isDepthPass = 0) const;


		// Getters & Setters
		const std::vector<Model*> GetModels() const { return m_pModels; }
		const std::vector<Mesh*> GetTransparentMeshes(const glm::vec3& cameraPos) const
		{
			std::vector<Mesh*> transparentMeshes;
			
			for (const auto& model : m_pModels) {
				const auto& meshes = model->GetTransparentMeshes();
				transparentMeshes.insert(transparentMeshes.end(), meshes.begin(), meshes.end());
			}
			
			std::sort(transparentMeshes.begin(), transparentMeshes.end(),
				[&](Mesh* a, Mesh* b)
				{

					float distA = glm::length(glm::vec3(a->GetTransform()[3]) - cameraPos);
					float distB = glm::length(glm::vec3(b->GetTransform()[3]) - cameraPos);
					return distA > distB; // back to front
				});

			return transparentMeshes;
		}
		const DirectionalLight& GetDirectionalLight() const { return m_DirectionalLight; }
		const std::vector<PointLight>& GetPointLights() const { return m_PointLights; }
		
	private:
		// Private members
		//--------------------
		Device& m_Device;
		UniformBuffer<MatrixUbo>* m_pUniformBuffer;
		
		std::vector<Model*> m_pModels;
		DirectionalLight m_DirectionalLight{};
		std::vector<PointLight> m_PointLights;
	};
}
