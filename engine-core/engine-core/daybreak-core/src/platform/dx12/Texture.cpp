#include "daybreak.h"

#include "Texture.h"

#include "ResourceStateTracker.h"

namespace dx12 {
	Texture::Texture(gfx::TextureType type, const std::wstring& name) :
		Resource(),
		m_textureType(type) {}
	
	Texture::Texture(const D3D12_RESOURCE_DESC& resourceDesc, const D3D12_CLEAR_VALUE* clearValue, gfx::TextureType type, const std::wstring& name) : 
		Resource(resourceDesc, clearValue, name),
		m_textureType(type) {
		CreateViews();
	}

	Texture::Texture(ComPtr<ID3D12Resource> resource, gfx::TextureType type, const std::wstring& name) : 
		Resource(resource, name),
		m_textureType(type) {
		CreateViews();
	}

	Texture::Texture(const Texture& copy) : Resource(copy) {
		CreateViews();
	}

	Texture::Texture(Texture&& copy) : Resource(copy) {
		CreateViews();
	}

	Texture& Texture::operator=(const Texture& other) {
		Resource::operator=(other);
		CreateViews();
		return *this;
	}

	Texture& Texture::operator=(Texture&& other) {
		Resource::operator=(other);
		CreateViews();
		return *this;
	}

	Texture::~Texture() {}

	void Texture::Resize(uint32_t width, uint32_t height, uint32_t depthOrArraySize) {
		if (m_resource) {
			ResourceStateTracker::RemoveGlobalResourceState(m_resource.Get());
			CD3DX12_RESOURCE_DESC resDesc(m_resource->GetDesc());

			resDesc.Width = std::max(width, 1u);
			resDesc.Height = std::max(height, 1u);
			resDesc.DepthOrArraySize = depthOrArraySize;

			auto device = Application::Device();

			auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			ThrowOnFailure(device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&resDesc,
				D3D12_RESOURCE_STATE_COMMON,
				m_clearValue.get(),
				IID_PPV_ARGS(&m_resource)
			));

			m_resource->SetName(m_name.c_str());
			ResourceStateTracker::AddGlobalResourceState(m_resource.Get(), D3D12_RESOURCE_STATE_COMMON);
			CreateViews();
		}
	}

	D3D12_UNORDERED_ACCESS_VIEW_DESC GetUAVDesc(const D3D12_RESOURCE_DESC& resDesc, UINT mipSlice, UINT arraySlice = 0, UINT planeSlice = 0) {
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = resDesc.Format;

		switch (resDesc.Dimension) {
			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
				if (resDesc.DepthOrArraySize > 1) {
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
					uavDesc.Texture1DArray.ArraySize = resDesc.DepthOrArraySize - arraySlice;
					uavDesc.Texture1DArray.FirstArraySlice = arraySlice;
					uavDesc.Texture1DArray.MipSlice = mipSlice;
				} else {
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
					uavDesc.Texture1D.MipSlice = mipSlice;
				}
				break;
			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
				if (resDesc.DepthOrArraySize > 1) {
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
					uavDesc.Texture2DArray.ArraySize = resDesc.DepthOrArraySize - arraySlice;
					uavDesc.Texture2DArray.FirstArraySlice = arraySlice;
					uavDesc.Texture2DArray.PlaneSlice = planeSlice;
					uavDesc.Texture2DArray.MipSlice = mipSlice;
				} else {
					uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
					uavDesc.Texture2D.PlaneSlice = planeSlice;
					uavDesc.Texture2D.MipSlice = mipSlice;
				}
				break;
			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
				uavDesc.Texture3D.WSize = resDesc.DepthOrArraySize - arraySlice;
				uavDesc.Texture3D.FirstWSlice = arraySlice;
				uavDesc.Texture3D.MipSlice = mipSlice;
				break;
			default:
				throw std::exception("Invalid resource dimension.");
		}

		return uavDesc;
	}


	void Texture::CreateViews() {
		if (m_resource) {
			auto app = Application::Get();
			auto device = Application::Device();

			CD3DX12_RESOURCE_DESC desc(m_resource->GetDesc());
			D3D12_FEATURE_DATA_FORMAT_SUPPORT formatSupport;
			formatSupport.Format = desc.Format;
			ThrowOnFailure(device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &formatSupport, sizeof(D3D12_FEATURE_DATA_FORMAT_SUPPORT)));

			if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0 && CheckRTVSupport(formatSupport.Support1)) {
				m_renderTargetView = app->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
				device->CreateRenderTargetView(m_resource.Get(), nullptr, m_renderTargetView.GetDescriptorHandle());
			}

			if ((desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0 && CheckDSVSupport(formatSupport.Support1)) {
				m_depthStencilView = app->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
				device->CreateDepthStencilView(m_resource.Get(), nullptr, m_depthStencilView.GetDescriptorHandle());
			}
		}

		std::lock_guard<std::mutex> lock(m_SRVmutex);
		std::lock_guard<std::mutex> guard(m_UAVmutex);

		m_SRVs.clear();
		m_UAVs.clear();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const {
		std::size_t hash = 0;
		if (srvDesc) {
			hash = std::hash<D3D12_SHADER_RESOURCE_VIEW_DESC>{}(*srvDesc);
		}

		std::lock_guard<std::mutex> lock(m_SRVmutex);
		auto iter = m_SRVs.find(hash);
		if (iter == m_SRVs.end()) {
			auto srv = CreateShaderResourceView(srvDesc);
			iter = m_SRVs.insert({ hash, std::move(srv) }).first;
		}

		return iter->second.GetDescriptorHandle();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const {
		std::size_t hash = 0;
		if (uavDesc) {
			hash = std::hash<D3D12_UNORDERED_ACCESS_VIEW_DESC>{}(*uavDesc);
		}

		std::lock_guard<std::mutex> guard(m_UAVmutex);
		auto iter = m_UAVs.find(hash);
		if (iter == m_UAVs.end()) {
			auto uav = CreateUnorderedAccessView(uavDesc);
			iter = m_UAVs.insert({ hash, std::move(uav) }).first;
		}

		return iter->second.GetDescriptorHandle();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetRenderTargetView() const {
		return m_renderTargetView.GetDescriptorHandle();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetDepthStencilView() const {
		return m_depthStencilView.GetDescriptorHandle();
	}

	bool Texture::IsUAVCompatibleFormat(DXGI_FORMAT format) {
		switch (format) {
			case DXGI_FORMAT_R32G32B32A32_FLOAT:
			case DXGI_FORMAT_R32G32B32A32_UINT:
			case DXGI_FORMAT_R32G32B32A32_SINT:
			case DXGI_FORMAT_R16G16B16A16_FLOAT:
			case DXGI_FORMAT_R16G16B16A16_UINT:
			case DXGI_FORMAT_R16G16B16A16_SINT:
			case DXGI_FORMAT_R8G8B8A8_UNORM:
			case DXGI_FORMAT_R8G8B8A8_UINT:
			case DXGI_FORMAT_R8G8B8A8_SINT:
			case DXGI_FORMAT_R32_FLOAT:
			case DXGI_FORMAT_R32_UINT:
			case DXGI_FORMAT_R32_SINT:
			case DXGI_FORMAT_R16_FLOAT:
			case DXGI_FORMAT_R16_UINT:
			case DXGI_FORMAT_R16_SINT:
			case DXGI_FORMAT_R8_UNORM:
			case DXGI_FORMAT_R8_UINT:
			case DXGI_FORMAT_R8_SINT:
				return true;
			default:
				return false;
		}
	}

	bool Texture::IsSRGBFormat(DXGI_FORMAT format) {
		switch (format) {
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			case DXGI_FORMAT_BC1_UNORM_SRGB:
			case DXGI_FORMAT_BC2_UNORM_SRGB:
			case DXGI_FORMAT_BC3_UNORM_SRGB:
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			case DXGI_FORMAT_BC7_UNORM_SRGB:
				return true;
			default:
				return false;
		}
	}

	bool Texture::IsBGRFormat(DXGI_FORMAT format) {
		switch (format) {
			case DXGI_FORMAT_B8G8R8A8_UNORM:
			case DXGI_FORMAT_B8G8R8X8_UNORM:
			case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			case DXGI_FORMAT_B8G8R8X8_TYPELESS:
			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
				return true;
			default:
				return false;
		}
	}

	bool Texture::IsDepthFormat(DXGI_FORMAT format) {
		switch (format) {
			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			case DXGI_FORMAT_D32_FLOAT:
			case DXGI_FORMAT_D24_UNORM_S8_UINT:
			case DXGI_FORMAT_D16_UNORM:
				return true;
			default:
				return false;
		}
	}

	DescriptorAllocation Texture::CreateShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const {
		auto app = Application::Get();
		auto device = Application::Device();
		auto srv = app->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		device->CreateShaderResourceView(m_resource.Get(), srvDesc, srv.GetDescriptorHandle());

		return srv;
	}

	DescriptorAllocation Texture::CreateUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const {
		auto app = Application::Get();
		auto device = Application::Device();
		auto uav = app->AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		device->CreateUnorderedAccessView(m_resource.Get(), nullptr, uavDesc, uav.GetDescriptorHandle());

		return uav;
	}
}