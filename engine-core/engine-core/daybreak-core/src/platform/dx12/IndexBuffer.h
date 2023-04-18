#pragma once

#include "Buffer.h"

namespace dx12 {

	class IndexBuffer : public Buffer {
	public:
        IndexBuffer(const std::wstring& name = L"");
        virtual ~IndexBuffer();

        virtual void CreateViews(size_t numElements, size_t elementSize) override;

        /**
        * Get the SRV for a resource.
        */
        virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const override;

        /**
        * Get the UAV for a (sub)resource.
        */
        virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const override;

        size_t NumIndicies() const { return m_nIndicies; }
        DXGI_FORMAT IndexFormat() const { return m_indexFormat; }
        D3D12_INDEX_BUFFER_VIEW IndexBufferView() const { return m_indexBufferView; }

	private:
		size_t					m_nIndicies;
		DXGI_FORMAT				m_indexFormat;
		D3D12_INDEX_BUFFER_VIEW	m_indexBufferView;
	};
}