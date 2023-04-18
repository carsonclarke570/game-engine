#pragma once

#include "platform/dx12/CommandList.h"
#include "platform/dx12/VertexBuffer.h"
#include "platform/dx12/IndexBuffer.h"

namespace gfx {
    struct DAYBREAK_API VertexData {

        VertexData() {}

        VertexData(const XMFLOAT3& position, const XMFLOAT3& normal, const XMFLOAT3& tangent, const XMFLOAT3& color, const XMFLOAT2& uv) :
            position(position),
            normal(normal),
            tangent(tangent),
            color(color),
            uv(uv)
        { }

        VertexData(FXMVECTOR position, FXMVECTOR normal, FXMVECTOR tangent, FXMVECTOR color, FXMVECTOR uv) {
            XMStoreFloat3(&this->position, position);
            XMStoreFloat3(&this->normal, normal);
            XMStoreFloat3(&this->tangent, tangent);
            XMStoreFloat3(&this->color, color);
            XMStoreFloat2(&this->uv, uv);
        }

        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT3 tangent;
        DirectX::XMFLOAT3 color;
        DirectX::XMFLOAT2 uv;

        static const int InputElementCount = 5;
        static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
    };

    

    class DAYBREAK_API Mesh {
        public:
            using Vertices = std::vector<VertexData>;
            using Indices = std::vector<uint16_t>;
            Mesh();
            virtual ~Mesh();

            static std::shared_ptr<Mesh> Create(dx12::CommandList& commandList, Vertices& vertices, Indices& indices);

            void Draw(dx12::CommandList& commandlist);
            // static std::unique_ptr<Mesh> LoadFromFile(const std::string& filePath);

            // static std::unique_ptr<Mesh> CreateCube(dx12::CommandList& commandList, FXMVECTOR color = {0.196f, 0.573f, 0.035}, float size = 1, bool rhcoords = false);
        private:
            friend struct std::default_delete<Mesh>;

            Mesh(const Mesh& copy) = delete;

            // void CreateBuffers();
            void Initialize(dx12::CommandList& commandList, Vertices& vertices, Indices& indices);

            dx12::VertexBuffer  m_vertexBuffer;
            dx12::IndexBuffer   m_indexBuffer;
            UINT                m_indexCount;
    };
}

