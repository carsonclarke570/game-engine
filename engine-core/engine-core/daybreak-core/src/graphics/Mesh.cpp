#include "daybreak.h"

#include "Mesh.h"

namespace gfx {

	const D3D12_INPUT_ELEMENT_DESC VertexData::InputElements[] = {
		{ "POSITION",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL",     0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "UV",			0, DXGI_FORMAT_R32G32_FLOAT,	0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	Mesh::Mesh() :
		m_indexCount(0) {}

	Mesh::~Mesh() {}

	std::shared_ptr<Mesh> Mesh::Create(dx12::CommandList& commandList, Vertices& vertices, Indices& indices) {
		std::shared_ptr<Mesh> mesh = std::make_shared<Mesh>();
		mesh->Initialize(commandList, vertices, indices);
		return mesh;
	}

	void Mesh::Draw(dx12::CommandList& commandlist) {
		commandlist.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandlist.SetVertexBuffer(0, m_vertexBuffer);
		commandlist.SetIndexBuffer(m_indexBuffer);
		commandlist.DrawIndexed(m_indexCount);
	}

	//std::unique_ptr<Mesh> Mesh::LoadFromFile(const std::string& filePath) {
	//	Assimp::Importer importer;
	//	const aiScene* scene = importer.ReadFile(
	//		filePath,
	//		aiProcess_CalcTangentSpace | 
	//		aiProcess_Triangulate | 
	//		aiProcess_JoinIdenticalVertices | 
	//		aiProcess_ValidateDataStructure |
	//		aiProcess_SortByPType | 
	//		aiProcess_ConvertToLeftHanded
	//	);

	//	if (!scene) {
	//		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	//		std::wstring errorString = converter.from_bytes(importer.GetErrorString());
	//		Logger::error(errorString.c_str());
	//	}

	//	return std::unique_ptr<Mesh>();
	//}

	//std::unique_ptr<Mesh> Mesh::CreateCube(dx12::CommandList& commandList, FXMVECTOR color, float size, bool rhcoords) {
 //       const int FaceCount = 6;

 //       static const XMVECTORF32 faceNormals[FaceCount] =
 //       {
 //           { 0,  0,  1 },	// AWAY 
 //           { 0,  0, -1 },	// TOWARD
 //           { 1,  0,  0 },	// RIGHT
 //           { -1,  0,  0 },	// LEFT
 //           { 0,  1,  0 },	// UP
 //           { 0, -1,  0 },	// DOWN
 //       };

	//	static const XMVECTORF32 textureCoordinates[4] = {
	//		{ 1, 0 },	// UPPER RIGHT
	//		{ 1, 1 },	// LOWER RIGHT
	//		{ 0, 1 },	// LOWER LEFT
	//		{ 0, 0 },	// UPPER LEFT
	//	};

 //       size /= 2;

 //       // Create the primitive object.
 //       std::unique_ptr<Mesh> mesh(new Mesh());

 //       // Create each face in turn.
 //       for (int i = 0; i < FaceCount; i++)
 //       {
 //           XMVECTOR normal = faceNormals[i];

 //           // Get two vectors perpendicular both to the face normal and to each other.
 //           XMVECTOR basis = (i >= 4) ? g_XMIdentityR2 : g_XMIdentityR1;

 //           XMVECTOR side1 = XMVector3Cross(normal, basis);
 //           XMVECTOR side2 = XMVector3Cross(normal, side1);

 //           // Six indices (two triangles) per face.
 //           size_t vbase = mesh->m_vertices.size();
 //           mesh->m_indices.push_back(static_cast<uint16_t>(vbase + 0));
 //           mesh->m_indices.push_back(static_cast<uint16_t>(vbase + 1));
 //           mesh->m_indices.push_back(static_cast<uint16_t>(vbase + 2));

 //           mesh->m_indices.push_back(static_cast<uint16_t>(vbase + 0));
 //           mesh->m_indices.push_back(static_cast<uint16_t>(vbase + 2));
 //           mesh->m_indices.push_back(static_cast<uint16_t>(vbase + 3));

 //           // Four vertices per face.
 //           mesh->m_vertices.push_back(VertexData((normal - side1 - side2) * size, normal, { 0.0f, 0.0f, 0.0f }, color, textureCoordinates[0]));
 //           mesh->m_vertices.push_back(VertexData((normal - side1 + side2) * size, normal, { 0.0f, 0.0f, 0.0f }, color, textureCoordinates[1]));
 //           mesh->m_vertices.push_back(VertexData((normal + side1 + side2) * size, normal, { 0.0f, 0.0f, 0.0f }, color, textureCoordinates[2]));
 //           mesh->m_vertices.push_back(VertexData((normal + side1 - side2) * size, normal, { 0.0f, 0.0f, 0.0f }, color, textureCoordinates[3]));
 //       }

	//	// mesh->CalculateTangents();
 //       // mesh->Initialize(commandList, false);

 //       return mesh;
	//}

	//static void ReverseWinding(Indices& indices, Vertices& vertices) {
	//	assert((indices.size() % 3) == 0);
	//	for (auto it = indices.begin(); it != indices.end(); it += 3) {
	//		std::swap(*it, *(it + 2));
	//	}

	//	for (auto it = vertices.begin(); it != vertices.end(); ++it) {
	//		it->uv.x = (1.f - it->uv.x);
	//	}
	//}

	
	void Mesh::Initialize(dx12::CommandList& commandList, Vertices& vertices, Indices& indices) {
		if (vertices.size() >= USHRT_MAX) {
			throw std::exception("Too many vertices for 16-bit index buffer");
		}

		commandList.CopyVertexBuffer(m_vertexBuffer, vertices);
		commandList.CopyIndexBuffer(m_indexBuffer, indices);

		m_indexCount = static_cast<UINT>(indices.size());
	}
}