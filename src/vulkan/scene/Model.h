#pragma once
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>



// std
#include <memory>
#include <string>
#include <algorithm>

namespace cat
{
	struct AABB {
		glm::vec3 min;
		glm::vec3 max;
	};

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
		void SetTransform(const glm::mat4& transform) { m_TransformMatrix = transform; }
		glm::mat4* GetTransform() { return &m_TransformMatrix; }
		void SetTranslation(const glm::vec3& translation) { m_TransformMatrix = glm::translate(m_TransformMatrix, translation); }
		void SetRotation(float angle, const glm::vec3& axis) { m_TransformMatrix = glm::rotate(m_TransformMatrix, angle, axis); }
		void SetScale(const glm::vec3& scale) { m_TransformMatrix = glm::scale(m_TransformMatrix, scale); }
		glm::vec3 GetWorldPosition() const { return glm::vec3(m_TransformMatrix[3]); }

		std::pair<glm::vec3, glm::vec3> GetBounds() const { return { m_MinBounds, m_MaxBounds }; }

		const std::vector<Mesh*>& GetOpaqueMeshes() const { return m_OpaqueMeshes; }
		const std::vector<Mesh*>& GetTransparentMeshes() const { return m_TransparentMeshes; }
		std::string GetPath() const { return m_Path; }


	private:
		// Private methods
		//--------------------
		void LoadModel(const std::string& path);
		void ProcessNode(::aiNode* node, const ::aiScene* scene, const glm::mat4& parentTransform);
		void ProcessMesh(::aiMesh* mesh, const ::aiScene* scene, const glm::mat4& transform);
		glm::mat4 ConvertMatrixToGLM(const aiMatrix4x4& mat) const;

		AABB CalculateAABB(const std::vector<Mesh::Vertex>& vertices) const
		{
			AABB box;
			box.min = glm::vec3(FLT_MAX);
			box.max = glm::vec3(-FLT_MAX);

			for (const auto& v : vertices) 
			{
				// min
				if (v.pos.x < box.min.x) box.min.x = v.pos.x;
				if (v.pos.y < box.min.y) box.min.y = v.pos.y;
				if (v.pos.z < box.min.z) box.min.z = v.pos.z;

				// max
				if (v.pos.x > box.max.x) box.max.x = v.pos.x;
				if (v.pos.y > box.max.y) box.max.y = v.pos.y;
				if (v.pos.z > box.max.z) box.max.z = v.pos.z;
			}
			return box;
		}

		// Private Datamembers
		//--------------------
		Device& m_Device;
		UniformBuffer<MatrixUbo>* m_pUniformBuffer;
		DescriptorSetLayout* m_pDescriptorSetLayout;
		DescriptorPool* m_pDescriptorPool;

		std::vector<Mesh*> m_OpaqueMeshes;
		std::vector<Mesh*> m_TransparentMeshes;
		std::vector<Mesh::RawMeshData> m_RawMeshes;
		std::vector<Mesh::Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
		Mesh::Material m_Material;
		std::string m_Path;
		std::string m_Directory;

		glm::mat4 m_TransformMatrix = glm::mat4(1);

		glm::vec3 m_MinBounds = glm::vec3(FLT_MAX);
		glm::vec3 m_MaxBounds = glm::vec3(-FLT_MAX);
	};
}