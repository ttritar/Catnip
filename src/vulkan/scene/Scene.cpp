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
		glm::vec3 sceneCenter = (m_MinBounds + m_MaxBounds) * 0.5f;
		glm::vec3 lightDirection = glm::normalize(m_DirectionalLight.direction);


		std::vector<glm::vec3> corners = {
			{ m_MinBounds.x, m_MinBounds.y, m_MinBounds.z },
			{ m_MaxBounds.x, m_MinBounds.y, m_MinBounds.z },
			{ m_MinBounds.x, m_MaxBounds.y, m_MinBounds.z },
			{ m_MaxBounds.x, m_MaxBounds.y, m_MinBounds.z },
			{ m_MinBounds.x, m_MinBounds.y, m_MaxBounds.z },
			{ m_MaxBounds.x, m_MinBounds.y, m_MaxBounds.z },
			{ m_MinBounds.x, m_MaxBounds.y, m_MaxBounds.z },
			{ m_MaxBounds.x, m_MaxBounds.y, m_MaxBounds.z },
		};

		float minProj = FLT_MAX;
		float maxProj = -FLT_MAX;

		for (const auto& corner : corners)
		{
			float proj = glm::dot(corner, lightDirection);
			minProj = std::min(minProj, proj);
			maxProj = std::max(maxProj, proj);
		}

		const float distance = maxProj - glm::dot(sceneCenter, lightDirection);
		const glm::vec3 lightPosition = sceneCenter - lightDirection * distance;

		const glm::vec3 up = glm::abs(glm::dot(lightDirection, glm::vec3(0.f, 1.f, 0.f))) < 0.999f
			? glm::vec3(0.f, 1.f, 0.f)
			: glm::vec3(1.f, 0.f, 0.f);

		m_DirectionalLight.viewMatrix = glm::lookAt(lightPosition, sceneCenter, up);

		glm::vec3 minLightSpace(FLT_MAX);
		glm::vec3 maxLightSpace(-FLT_MAX);
		for (const auto& corner : corners)
		{
			const glm::vec3 transformedCorner = glm::vec3(m_DirectionalLight.viewMatrix * glm::vec4(corner, 1.0f));
			minLightSpace = glm::min(minLightSpace, transformedCorner);
			maxLightSpace = glm::max(maxLightSpace, transformedCorner);
		}

		const float nearZ = 0.0f;
		const float farZ = maxLightSpace.z - minLightSpace.z;
		m_DirectionalLight.projectionMatrix = glm::ortho(
			minLightSpace.x, maxLightSpace.x,
			minLightSpace.y, maxLightSpace.y,
			nearZ, farZ
		);

	}

	Model* Scene::AddModel(const std::string& path)
	{
		Model* model = new Model(m_Device,m_pUniformBuffer,path);
		m_pModels.push_back(model);

		auto [modelMin, modelMax] = model->GetBounds();
		m_MinBounds = glm::min(m_MinBounds, modelMin);
		m_MaxBounds = glm::max(m_MaxBounds, modelMax);

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

	void Scene::DrawOpaque(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t frameIdx,
		bool isDepthPass) const
	{

		for (const auto& model : m_pModels)
		{
			vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), model->GetTransform());
			for (auto mesh : model->GetOpaqueMeshes())
			{
				mesh->Bind(commandBuffer, pipelineLayout, frameIdx, isDepthPass);
				mesh->Draw(commandBuffer);
			}
		}
	}
}
