#include "daybreak.h"
#include "Model.h"

#include "Mesh.h"

namespace gfx {
	
	Model::Model() : m_meshes() {}

	Model::~Model() {}

	std::shared_ptr<Model> Model::LoadFromFile(dx12::CommandList& commandList, const std::string& file) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(
			file,
			aiProcess_CalcTangentSpace | 
			aiProcess_Triangulate | 
			aiProcess_JoinIdenticalVertices | 
			aiProcess_ValidateDataStructure |
			aiProcess_SortByPType | 
			aiProcess_ConvertToLeftHanded
		);

		if (!scene) {
			throw std::exception(importer.GetErrorString());
		}

		std::shared_ptr<Model> model = std::make_shared<Model>();
		model->ProcessAINode(commandList, scene->mRootNode, scene);
		return model;
	}

	void Model::Draw(dx12::CommandList& commandList) {
		for (int i = 0; i < m_meshes.size(); i++) {
			m_meshes[i]->Draw(commandList);
		}
	}

	void Model::ProcessAINode(dx12::CommandList& commandList, aiNode* node, const aiScene* scene) {
		for (int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			m_meshes.push_back(ProcessAIMesh(commandList, mesh, scene));
		}

		for (int i = 0; i < node->mNumChildren; i++) {
			ProcessAINode(commandList, node->mChildren[i], scene);
		}
	}

	std::shared_ptr<Mesh> Model::ProcessAIMesh(dx12::CommandList& commandList, aiMesh* mesh, const aiScene* scene) {
		Mesh::Vertices vertices;
		Mesh::Indices indices;

		for (int i = 0; i < mesh->mNumVertices; i++) {
			VertexData data;
			
			// COLOR -- TODO: Material handling
			if (mesh->HasVertexColors(i)) {
				if (i == 0) {
					Logger::error(L"Vertex color unsupported -- defaulting\n");
				}
				data.color = { 0.196f, 0.573f, 0.035 };
			} else {
				data.color = { 0.196f, 0.573f, 0.035 };
			}
			
			// POSITION
			if (mesh->HasPositions()) {
				aiVector3D meshVertex = mesh->mVertices[i];
				data.position.x = meshVertex.x;
				data.position.y = meshVertex.y;
				data.position.z = meshVertex.z;
			} else {
				Logger::error(L"Invalid mesh!\n");
				throw std::exception("Invalid mesh!");
			}

			// NORMAL
			if (mesh->HasNormals()) {
				aiVector3D meshNormal = mesh->mNormals[i];
				data.normal.x = meshNormal.x;
				data.normal.y = meshNormal.y;
				data.normal.z = meshNormal.z;
			} else {
				data.normal = { 0.0f, 0.0f, 0.0f };
			}
		
			// TEXTURE COORD
			if (mesh->HasTextureCoords(i)) {
				aiVector3D meshUV = mesh->mTextureCoords[0][i];
				data.uv.x = meshUV.x;
				data.uv.y = meshUV.y;
			} else {
				data.uv = { 0.0f, 0.0f };
			}

			// TANGENTS
			if (mesh->HasTangentsAndBitangents()) {
				aiVector3D meshTangent = mesh->mTangents[i];
				data.tangent.x = meshTangent.x;
				data.tangent.y = meshTangent.y;
				data.tangent.z = meshTangent.z;
			} else {
				data.tangent = { 0.0f, 0.0f, 0.0f };
			}

			vertices.push_back(data);
		}

		for (int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (int j = 0; j < face.mNumIndices; j++) {
				indices.push_back(face.mIndices[j]);
			}
		}

		return gfx::Mesh::Create(commandList, vertices, indices);
	}
}