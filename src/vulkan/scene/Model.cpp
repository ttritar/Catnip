#include "Model.h"

#include <iostream>

#undef min
#undef max

namespace cat
{
	// CTOR & DTOR
	//--------------------

	Model::Model(Device& device, UniformBuffer<MatrixUbo>* ubo, const std::string& path)
		: m_Device{ device },
		m_pUniformBuffer{ ubo },
		m_Path(path), m_Directory{ path }
	{
		LoadModel(path);

		// Create descriptor pool
		m_pDescriptorPool = new DescriptorPool(device);
		m_pDescriptorPool
			->AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(m_RawMeshes.size() * cat::MAX_FRAMES_IN_FLIGHT))
			->AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				static_cast<uint32_t>(m_RawMeshes.size() * cat::MAX_FRAMES_IN_FLIGHT) * m_Material.amount);
		m_pDescriptorPool = m_pDescriptorPool->Create(static_cast<uint32_t>(m_RawMeshes.size() * cat::MAX_FRAMES_IN_FLIGHT));

		// Create descriptor set layout
		m_pDescriptorSetLayout = new DescriptorSetLayout(device);
		m_pDescriptorSetLayout = m_pDescriptorSetLayout
			->AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VK_SHADER_STAGE_FRAGMENT_BIT) // albedo sampler
			->AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VK_SHADER_STAGE_FRAGMENT_BIT) // normal sampler
			->AddBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				VK_SHADER_STAGE_FRAGMENT_BIT) // specular sampler
			->Create();

		// Create meshes
		for (auto& data : m_RawMeshes)
		{
			if (data.opaque)
			{
				m_OpaqueMeshes.push_back(new Mesh(m_Device, ubo,
					m_pDescriptorSetLayout, m_pDescriptorPool,
					data));
			}
			else
			{
				m_TransparentMeshes.push_back(new Mesh(m_Device, ubo,
					m_pDescriptorSetLayout, m_pDescriptorPool,
					data));
			}
		}

		m_RawMeshes.clear();
	}

	Model::~Model()
	{
		for (auto mesh : m_OpaqueMeshes)
		{
			delete mesh;
			mesh = nullptr;
		}

		for (auto mesh : m_TransparentMeshes)
		{
			delete mesh;
			mesh = nullptr;
		}

		delete m_pDescriptorPool;
		m_pDescriptorPool = nullptr;

		delete m_pDescriptorSetLayout;
		m_pDescriptorSetLayout = nullptr;
	}

	// Methods
	//--------------------
	void Model::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint16_t frameIdx, bool isDepthPass) const
	{
		for (auto& mesh : m_OpaqueMeshes)
		{
			mesh->Bind(commandBuffer, pipelineLayout, frameIdx, isDepthPass);
			mesh->Draw(commandBuffer);
		}

		for (auto& mesh : m_TransparentMeshes)
		{
			mesh->Bind(commandBuffer, pipelineLayout, frameIdx, isDepthPass);
			mesh->Draw(commandBuffer);
		}
	}

	// Private methods
	//--------------------
	void Model::LoadModel(const std::string& path)
	{
		// reset bound
		m_MinBounds = glm::vec3(std::numeric_limits<float>::max());
		m_MaxBounds = glm::vec3(std::numeric_limits<float>::lowest());


		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path,
			aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_ConvertToLeftHanded
		);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			std::cerr << "ERROR::ASSIMP::" << std::string(importer.GetErrorString()) << std::endl;
			return;
		}
		m_Directory = path.substr(0, path.find_last_of('/'));

		ProcessNode(scene->mRootNode, scene, m_TransformMatrix);
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene, const glm::mat4& parentTransform)
	{
		glm::mat4 localTransform = ConvertMatrixToGLM(node->mTransformation);
		glm::mat4 globalTransform = parentTransform * localTransform;

		// for each mesh in the node
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			ProcessMesh(mesh, scene, globalTransform);
		}

		// for each of its children
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			ProcessNode(node->mChildren[i], scene, globalTransform);
		}
	}

	void Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, const glm::mat4& transform)
	{
		std::vector<Mesh::Vertex> vertices;
		std::vector<uint32_t> indices;
		Mesh::Material material;
		bool opaque = true;


		// process vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Mesh::Vertex vertex;
			glm::vec3 vector;

			// positions
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			glm::vec4 transformedPos = transform * glm::vec4(vector, 1.0f);
			vertex.pos = glm::vec3(transformedPos.x, transformedPos.y, transformedPos.z);

			m_MinBounds = glm::min(m_MinBounds, vertex.pos);
			m_MaxBounds = glm::max(m_MaxBounds, vertex.pos);


			// colors
			if (mesh->HasVertexColors(i))
			{
				vector.x = mesh->mColors[i][i].r;
				vector.y = mesh->mColors[i][i].g;
				vector.z = mesh->mColors[i][i].b;
				vertex.color = vector;
			}
			else
			{
				vertex.color = { 1.0f, 1.0f, 1.0f };
			}

			// uvs
			if (mesh->HasTextureCoords(0))
			{
				// uv
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.uv = vec;
			}
			else
			{
				vertex.uv = { 0.0f, 0.0f };
			}

			// normals
			if (mesh->HasNormals())
			{
				glm::vec3 normal(
					mesh->mNormals[i].x,
					mesh->mNormals[i].y,
					mesh->mNormals[i].z
				);

				glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));
				vertex.normal = glm::normalize(normalMatrix * normal);
			}

			// tangents and bitangents
			if (mesh->HasTangentsAndBitangents())
			{
				// uv
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.uv = vec;

				// tangents and bitangents
				glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

				vertex.tangent = glm::normalize(normalMatrix * glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z));
				vertex.bitangent = glm::normalize(normalMatrix * glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z));

			}

			vertices.push_back(vertex);

			aiAABB aabb = mesh->mAABB;
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
		if (scene->mNumMaterials > 0 && mesh->mMaterialIndex >= 0) {
			aiMaterial* aiMaterial = scene->mMaterials[mesh->mMaterialIndex];
			aiString path;

			if (aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
				material.albedoPath = m_Directory + "/" + path.C_Str();
			else
				material.albedoPath = "";

			if (aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS)
				material.normalPath = m_Directory + "/" + path.C_Str();
			else
				material.normalPath = "";

			if (aiMaterial->GetTexture(aiTextureType_METALNESS, 0, &path) == AI_SUCCESS ||
				aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &path) == AI_SUCCESS)
				material.specularPath = m_Directory + "/" + path.C_Str();
			else
				material.specularPath = "";

			// transparency check
			float opacity = 1.0f;
			if (aiMaterial->GetTextureCount(aiTextureType_OPACITY) > 0 ||
				(aiMaterial->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS && opacity < 1.0f) ||
				material.albedoPath.ends_with(".png"))
			{
				opaque = false;
			}
		}


		m_RawMeshes.emplace_back(Mesh::RawMeshData{
			vertices,
			indices,
			material,
			transform,
			opaque
			});


	}

	glm::mat4 Model::ConvertMatrixToGLM(const aiMatrix4x4& mat) const
	{
		return glm::mat4(
			mat.a1, mat.b1, mat.c1, mat.d1,
			mat.a2, mat.b2, mat.c2, mat.d2,
			mat.a3, mat.b3, mat.c3, mat.d3,
			mat.a4, mat.b4, mat.c4, mat.d4
		);
	}

}