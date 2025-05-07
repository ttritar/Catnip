#include "Model.h"

#include <iostream>

namespace cat
{
	// CTOR & DTOR
	//--------------------

	Model::Model(Device& device,SwapChain& swapchain, const std::string& path)
		: m_Device{ device }, m_SwapChain{ swapchain }, m_Directory{ path }
	{
		LoadModel(path);
	}

	Model::~Model()
	{
		for (auto mesh : m_Meshes)
		{
			delete mesh;
			mesh = nullptr;
		}
	}

	// Methods
	//--------------------

	void Model::Bind(VkCommandBuffer commandBuffer) const
	{
		for (auto& mesh : m_Meshes)
		{
			mesh->Bind(commandBuffer);
		}
	}


	void Model::Draw(VkCommandBuffer commandBuffer) const
	{
		for (auto& mesh : m_Meshes)
		{
			mesh->Draw(commandBuffer);
		}
	}

	// Private methods
	//--------------------
	void Model::LoadModel(const std::string& path)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cerr<<"ERROR::ASSIMP::" << std::string(importer.GetErrorString())<<std::endl;
			return;
		}
		m_Directory = path.substr(0, path.find_last_of('/'));

		ProcessNode(scene->mRootNode, scene);
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene)
	{
		// for each mesh in the node
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_Meshes.emplace_back(new Mesh(ProcessMesh(mesh, scene)));
		}

		// for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene);
		}
	}

	Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) const
	{
		std::vector<Mesh::Vertex> vertices;
		std::vector<uint32_t> indices;
		std::vector<Image> images;

		// process vertices
		for (unsigned int i = 0; i< mesh->mNumVertices ; i++)
		{
			Mesh::Vertex vertex;
			glm::vec3 vector;

			// positions
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.pos = vector;

			// normals
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.color = vector;

			// uv
			if (mesh->mTextureCoords[0]) 
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x; 
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.uv = vec;
			}
			else
			{
				vertex.uv = { 0.0f, 0.0f };
			}

			vertices.push_back(vertex);
		}

		// process indices
		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		// process materials
		if (mesh->mMaterialIndex>=0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			
		}

		return Mesh(m_Device, vertices, indices, images);;
	}
}
