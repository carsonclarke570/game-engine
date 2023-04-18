#pragma once

#include "Buffer.h"

namespace dx12 {

	class VertexBuffer : public Buffer {
	    public:

            VertexBuffer(const std::wstring& name = L"");
            virtual ~VertexBuffer();
            virtual void CreateViews(size_t numElements, size_t elementSize) override;

            /**
             * Get the SRV for a resource.
             */
            virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const override;

            /**
            * Get the UAV for a (sub)resource.
            */
            virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const override;


            /**
             * Get the vertex buffer view for binding to the Input Assembler stage.
             */
            D3D12_VERTEX_BUFFER_VIEW VertexBufferView() const {
                return m_vertexBufferView;
            }

            size_t NumVertices() const {
                return m_nVertices;
            }

            size_t VertexStride() const {
                return m_vertexStride;
            }

       
        private:
		    size_t						m_nVertices;
		    size_t						m_vertexStride;
		    D3D12_VERTEX_BUFFER_VIEW	m_vertexBufferView;
	};
}