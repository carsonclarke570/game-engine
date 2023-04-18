#pragma once

namespace dx12 {

	class Resource {
	public:
		Resource(const std::wstring& name = L"");
		Resource(
			const D3D12_RESOURCE_DESC& resourceDesc,
			const D3D12_CLEAR_VALUE* clearValue = nullptr,
			const std::wstring& name = L""
		);
		Resource(ComPtr<ID3D12Resource> resource, const std::wstring& name = L"");

		Resource(const Resource& copy);
		Resource(Resource&& copy);

		Resource& operator=(const Resource& other);
		Resource& operator=(Resource&& other);

		virtual ~Resource();

		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const = 0;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const = 0;
		virtual void SetResource(
			ComPtr<ID3D12Resource> resource,
			const D3D12_CLEAR_VALUE* clearValue = nullptr
		);

		void SetName(const std::wstring& name);
		virtual void Reset();

		bool IsValid() const { return m_resource != nullptr; }
		ComPtr<ID3D12Resource> Get() const { return m_resource; }

		D3D12_RESOURCE_DESC ResourceDesc() const;

	protected:
		ComPtr<ID3D12Resource>				m_resource;
		std::unique_ptr<D3D12_CLEAR_VALUE>	m_clearValue;
		std::wstring						m_name;
	};
}