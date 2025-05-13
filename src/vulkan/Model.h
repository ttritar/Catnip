#pragma once
#include "Image.h"
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
		Model(Device& device, SwapChain& swapchain, const std::string& path, 
			DescriptorSetLayout& layout, DescriptorPool& pool);
		~Model();

		Model(const Model&) = delete;
		Model& operator=(const Model&) = delete;
		Model(Model&&) = delete;
		Model& operator=(Model&&) = delete;


		// Methods
		//--------------------
		void UpdateUniformBuffer(uint32_t currentImage, const glm::mat4& view, const glm::mat4& proj) const;
		void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet descriptorSet) const;

		// Getters & Setters
		void SetTransform(const glm::mat4& transform){ m_TransformMatrix = transform; }
		glm::mat4* GetTransform() { return &m_TransformMatrix; }
		void SetTranslation(const glm::vec3& translation) { m_TransformMatrix = glm::translate(m_TransformMatrix, translation); }
		void SetRotation(float angle, const glm::vec3& axis) { m_TransformMatrix = glm::rotate(m_TransformMatrix, angle, axis); }
		void SetScale(const glm::vec3& scale) { m_TransformMatrix = glm::scale(m_TransformMatrix, scale); }


		const std::vector<Mesh*>& GetMeshes() const { return m_Meshes; }
		std::string GetPath() const { return m_Path; }
		const std::vector<Image*> GetImages() const
		{
			std::vector<Image*> images;
			for (auto& mesh : m_Meshes)
			{
				const auto& meshImages = mesh->GetImages();
				images.insert(images.end(), meshImages.begin(), meshImages.end());
			}
			return images;
		}


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
		std::vector<Mesh::Material> m_LoadedTextures;
		std::string m_Path;
		std::string m_Directory;

		glm::mat4 m_TransformMatrix = glm::mat4(1);

		DescriptorSet* m_pDescriptorSet;
		UniformBuffer* m_pUniformBuffer;
	};
}
