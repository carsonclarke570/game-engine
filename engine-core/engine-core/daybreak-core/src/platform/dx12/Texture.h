#pragma once

#include "Resource.h"
#include "DescriptorAllocation.h"
#include "graphics/TextureType.h"

namespace dx12 {

	class DAYBREAK_API Texture : public Resource {
	public:
		explicit Texture(
			gfx::TextureType type		= gfx::TextureType::ALBEDO, 
			const std::wstring& name	= L""
		);
		explicit Texture(
			const D3D12_RESOURCE_DESC& resourceDesc,
			const D3D12_CLEAR_VALUE* clearValue		= nullptr,
			gfx::TextureType type					= gfx::TextureType::ALBEDO,
			const std::wstring& name				= L""
		);
		explicit Texture(
			ComPtr<ID3D12Resource> resource,
			gfx::TextureType type		= gfx::TextureType::ALBEDO,
			const std::wstring& name	= L""
		);

		Texture(const Texture& copy);
		Texture(Texture&& copy);

		Texture& operator=(const Texture& other);
		Texture& operator=(Texture&& other);

		virtual ~Texture();

		void Resize(uint32_t width, uint32_t height, uint32_t depthOrArraySize = 1);
		virtual void CreateViews();
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const override;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const override;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const;
		virtual D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const;

		gfx::TextureType Type() const { return m_textureType; }
		void SetType(gfx::TextureType type) { m_textureType = type; }

		static bool CheckSRVSupport(D3D12_FORMAT_SUPPORT1 formatSupport) {
			return ((formatSupport & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE) != 0 ||
				(formatSupport & D3D12_FORMAT_SUPPORT1_SHADER_LOAD) != 0);
		}

		static bool CheckRTVSupport(D3D12_FORMAT_SUPPORT1 formatSupport) {
			return ((formatSupport & D3D12_FORMAT_SUPPORT1_RENDER_TARGET) != 0);
		}

		static bool CheckUAVSupport(D3D12_FORMAT_SUPPORT1 formatSupport) {
			return ((formatSupport & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW) != 0);
		}

		static bool CheckDSVSupport(D3D12_FORMAT_SUPPORT1 formatSupport) {
			return ((formatSupport & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL) != 0);
		}

		static bool IsUAVCompatibleFormat(DXGI_FORMAT format);
		static bool IsSRGBFormat(DXGI_FORMAT format);
		static bool IsBGRFormat(DXGI_FORMAT format);
		static bool IsDepthFormat(DXGI_FORMAT format);

	private:
		DescriptorAllocation CreateShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const;
		DescriptorAllocation CreateUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const;
	
		mutable std::unordered_map<size_t, DescriptorAllocation> m_SRVs;
		mutable std::unordered_map<size_t, DescriptorAllocation> m_UAVs;

		mutable std::mutex m_SRVmutex;
		mutable std::mutex m_UAVmutex;

		DescriptorAllocation m_renderTargetView;
		DescriptorAllocation m_depthStencilView;

		gfx::TextureType m_textureType;
	};
}