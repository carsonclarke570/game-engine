#pragma once

#include "DescriptorAllocation.h"
#include "RenderTarget.h"

namespace dx12 {

	class Texture;

	class DAYBREAK_API Application {

		public:
			static void CreateApplication(std::wstring windowTitle, int initialWidth, int initialHeight, HWND windowHandle);
			static void DestroyApplication();

			static Application* Get();
			static bool IsInitialized();
			static ComPtr<ID3D12Device2> Device();

			void Resize(int width, int height);
			uint32_t Present(const Texture& texture);
			void Flush();

			DescriptorAllocation AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors = 1);
			void ReleaseStaleDescriptors(uint64_t finishedFrame);

			DXGI_SAMPLE_DESC GetMultisampleQualityLevels(DXGI_FORMAT format, UINT numSamples, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE) const;

			std::shared_ptr<CommandQueue> CommandQueue(D3D12_COMMAND_LIST_TYPE type) { return m_context.GetCommandQueue(type); }
			Context& GetContext() { return m_context; };
			int CurrentBackBufferIndex() { return m_currentBackBuffer; }
			const Texture& CurrentBackBuffer() const { return m_backBufferTextures[m_currentBackBuffer]; }
			std::shared_ptr<RenderTarget> GetRenderTarget() const { return m_renderTarget; }

		private:
			// DX12Components
			uint64_t							m_heapSize;
			Context								m_context;
			ComPtr<IDXGISwapChain4>				m_swapchain;

			// Frame & Fence State
			int									m_nFrames;
			int									m_currentBackBuffer;
			std::vector<Texture>				m_backBufferTextures;
			std::vector<uint64_t>				m_frameFenceValues;
			std::shared_ptr<RenderTarget>		m_renderTarget;

			// Other
			bool								m_useVSync;

			Application(std::wstring windowTitle, int nFrames);
			virtual ~Application();

			void Initialize(int initialWidth, int initialHeight, HWND windowHandle);
			void Destroy();

			void TransitionResource(
				ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource> resource,
				D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState
			);

			void UpdateRenderTargetViews();
			ComPtr<IDXGISwapChain4> CreateSwapChain(int width, int height, HWND windowHandle);
	};
}

