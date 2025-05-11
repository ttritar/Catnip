#pragma once
#include "Image.h"
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// std
#include <string>

#include "glm/ext/matrix_transform.hpp"

namespace cat
{
	class Model final
	{
	public:
		struct ModelUBO {
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 proj;
		};

		// CTOR & DTOR
		//--------------------
		Model(Device& device, SwapChain& swapchain, const std::string& path);
		~Model();

		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;
		Model(Model&&) = delete;
		Model& operator=(Model&&) = delete;


		// Methods
		//--------------------
		void Draw(VkCommandBuffer commandBuffer) const;

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
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene) const;

		// Private Datamembers
		//--------------------
		Device& m_Device;
		SwapChain& m_SwapChain;

		std::vector<Mesh*> m_Meshes;
		std::vector<Image> m_Images;
		std::string m_Path;
		std::string m_Directory;

		glm::mat4 m_TransformMatrix = glm::mat4(1);
	};
}
