#pragma once
#include "Image.h"
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// std
#include <string>

namespace cat
{
	class Model final
	{
	public:
		// CTOR & DTOR
		//--------------------
		Model(Device& device, SwapChain& swapchain, const std::string& path);
		~Model();


		// Methods
		//--------------------
		void Bind(VkCommandBuffer commandBuffer) const;
		void Draw(VkCommandBuffer commandBuffer) const;

		// Getters & Setters
		const std::vector<Mesh*>& GetMeshes() const { return m_Meshes; }


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
		std::string m_Directory;
	};
}
