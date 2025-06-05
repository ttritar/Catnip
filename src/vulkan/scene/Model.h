#pragma once
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// std
#include <memory>
#include <string>

namespace cat
{
	class Model final
	{
	public:

		// CTOR & DTOR
		//--------------------
		Model(Device& device, UniformBuffer<MatrixUbo>* ubo, const std::string& path);
		~Model();

		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;
		Model(Model&&) = delete;
		Model& operator=(Model&&) = delete;


		// Methods
		//--------------------
		void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t frameIdx, bool isDepthPass) const;

		// Getters & Setters
		void SetTransform(const glm::mat4& transform){ m_TransformMatrix = transform; }
		glm::mat4* GetTransform() { return &m_TransformMatrix; }
		void SetTranslation(const glm::vec3& translation) { m_TransformMatrix = glm::translate(m_TransformMatrix, translation); }
		void SetRotation(float angle, const glm::vec3& axis) { m_TransformMatrix = glm::rotate(m_TransformMatrix, angle, axis); }
		void SetScale(const glm::vec3& scale) { m_TransformMatrix = glm::scale(m_TransformMatrix, scale); }


		const std::vector<Mesh*>& GetMeshes() const { return m_Meshes; }
		std::string GetPath() const { return m_Path; }

	private:
		// Private methods
		//--------------------
		void LoadModel(const std::string& path);
		void ProcessNode(aiNode* node, const aiScene* scene);
		void ProcessMesh(aiMesh* mesh, const aiScene* scene);

		// Private Datamembers
		//--------------------
		Device& m_Device;
		UniformBuffer<MatrixUbo>* m_pUniformBuffer;
		DescriptorSetLayout* m_pDescriptorSetLayout;
		DescriptorPool* m_pDescriptorPool;

		std::vector<Mesh*> m_Meshes;
		std::vector<Mesh::RawMeshData> m_RawMeshes;
		std::vector<Mesh::Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		Mesh::Material m_Material;
		std::string m_Path;
		std::string m_Directory;

		glm::mat4 m_TransformMatrix = glm::mat4(1);
	};
}
