#include "daybreak.h"

#include "Resource.h"
#include "ResourceStateTracker.h"

namespace dx12 {
	Resource::Resource(const std::wstring& name) 
		: m_name(name) {}

	Resource::Resource(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue, const std::wstring& name) {
		auto device = Application::Device();
		if (clearValue) {
			m_clearValue = std::make_unique<D3D12_CLEAR_VALUE>(*clearValue);
		}

		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		ThrowOnFailure(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_COMMON,
			m_clearValue.get(),
			IID_PPV_ARGS(&m_resource)
		));

		ResourceStateTracker::AddGlobalResourceState(m_resource.Get(), D3D12_RESOURCE_STATE_COMMON);
		SetName(name);
	}

	Resource::Resource(ComPtr<ID3D12Resource> resource, const std::wstring& name) : 
		m_resource(resource) {
		SetName(name);
	}

	Resource::Resource(const Resource& copy) : 
		m_resource(copy.m_resource), 
		m_name(copy.m_name), 
		m_clearValue(std::make_unique<D3D12_CLEAR_VALUE>(*copy.m_clearValue)) {
		int i = 3;
	}

	Resource::Resource(Resource&& copy) :
		m_resource(std::move(copy.m_resource)), 
		m_name(std::move(copy.m_name)), 
		m_clearValue(std::move(copy.m_clearValue)) {
	}

	Resource& Resource::operator=(const Resource& other) {
		if (this != &other) {
			m_resource = other.m_resource;
			m_name = other.m_name;
			if (other.m_clearValue) {
				m_clearValue = std::make_unique<D3D12_CLEAR_VALUE>(*other.m_clearValue);
			}
		}
		return *this;
	}

	Resource& Resource::operator=(Resource&& other) {
		if (this != &other) {
			m_resource = other.m_resource;
			m_name = other.m_name;
			m_clearValue = std::move(other.m_clearValue);

			other.m_resource.Reset();
			other.m_name.clear();
		}

		return *this;
	}

	Resource::~Resource() { }

	void Resource::SetResource(ComPtr<ID3D12Resource> resource, const D3D12_CLEAR_VALUE* clearValue) {
		m_resource = resource;
		if (m_clearValue) {
			m_clearValue = std::make_unique<D3D12_CLEAR_VALUE>(*clearValue);
		} else {
			m_clearValue.reset();
		}
		SetName(m_name);
	}

	void Resource::SetName(const std::wstring& name) {
		m_name = name;
		if (m_resource && !m_name.empty()) {
			m_resource->SetName(m_name.c_str());
		}
	}

	void Resource::Reset() {
		m_resource.Reset();
		m_clearValue.reset();
	}

	D3D12_RESOURCE_DESC Resource::ResourceDesc() const {
		D3D12_RESOURCE_DESC resDesc = {};
		if (m_resource) {
			resDesc = m_resource->GetDesc();
		}

		return resDesc;
	}
}