#pragma once

namespace gfx {

	class Mesh;

	class DAYBREAK_API Model {
	public:
		static std::shared_ptr<Model> LoadFromFile(dx12::CommandList& commandList, const std::string& file);

		Model();
		virtual ~Model();
		void Draw(dx12::CommandList& commandList);

	private:
		friend struct std::default_delete<Model>;

		
		Model(const Model& copy) = delete;
		
		void ProcessAINode(dx12::CommandList& commandList, aiNode* node, const aiScene* scene);
		std::shared_ptr<Mesh> ProcessAIMesh(dx12::CommandList& commandList, aiMesh* mesh, const aiScene* scene);

		using ModelMeshes = std::vector<std::shared_ptr<Mesh>>;
		ModelMeshes m_meshes;
	};
}