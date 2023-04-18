#include "daybreak.h"

#include "Context.h"
#include "DescriptorAllocator.h"
#include "Texture.h"
#include "CommandList.h"

#include "engine/manager/FPSCounter.h"

#include "ResourceStateTracker.h"

namespace dx12 {

	static Application* g_application = nullptr;
	static bool g_applicationInitialized = false;

	Application::Application(std::wstring windowTitle, int nFrames) :
		m_heapSize(0),
		m_context(),
		m_swapchain(nullptr),
		m_nFrames(nFrames),
		m_currentBackBuffer(0),
		m_backBufferTextures(nFrames),
		m_frameFenceValues(nFrames),
		m_useVSync(TRUE) {
		m_renderTarget = std::make_shared<RenderTarget>();
	}
	
	Application::~Application() { 
		Destroy();
	}

	void Application::CreateApplication(std::wstring windowTitle, int initialWidth, int initialHeight, HWND windowHandle) {
		if (!g_application) {
			Logger::info(L"[Application] Creating global Application instance...\n");
			g_application = new Application(windowTitle, 3);
			g_application->Initialize(initialWidth, initialHeight, windowHandle);
		}
	}

	void Application::DestroyApplication() {
		if (g_application) {
			Logger::info(L"[Application] Destorying global Application instance...\n");
			delete g_application;
			g_application = nullptr;

			g_applicationInitialized = false;
		}
	}

	Application* Application::Get() {
		return g_application;
	}

	bool Application::IsInitialized() {
		return g_applicationInitialized;
	}

	ComPtr<ID3D12Device2> Application::Device() {
		if (!g_application || !IsInitialized()) {
			Logger::error(L"[Application] No application instantiated!\n");
			throw std::exception("No device!");
		}

		return g_application->GetContext().Device();
	}

	void Application::Destroy() {
		Flush();
	}

	void Application::Initialize(int initialWidth, int initialHeight, HWND windowHandle) {
		m_context.Create();

		Logger::info(L"[Application] Setting up backbuffers...\n");
		for (int i = 0; i < m_nFrames; i++) {
			m_backBufferTextures[i].SetName(L"Backbuffer[" + std::to_wstring(i) + L"]");
		}

		Logger::info(L"[Application] Creating swap chain...\n");
		ID3D12CommandQueue* queue = m_context.GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->D3D12CommandQueue().Get();
		m_swapchain = CreateSwapChain(initialWidth, initialHeight, windowHandle);
		m_currentBackBuffer = m_swapchain->GetCurrentBackBufferIndex();

		g_applicationInitialized = true;

		UpdateRenderTargetViews();
	}

	void Application::Resize(int width, int height) {
		Flush();

		// Release references
		m_renderTarget->Release();
		for (int i = 0; i < m_nFrames; i++) {
			ResourceStateTracker::RemoveGlobalResourceState(m_backBufferTextures[i].Get().Get());
			m_backBufferTextures[i].Reset();
		}

		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		ThrowOnFailure(m_swapchain->GetDesc(&swapChainDesc));
		ThrowOnFailure(m_swapchain->ResizeBuffers(m_nFrames, width,
			height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags)
		);
		m_currentBackBuffer = m_swapchain->GetCurrentBackBufferIndex();

		UpdateRenderTargetViews();
	}

	uint32_t Application::Present(const Texture& texture) {
		auto commandQueue = CommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
		auto commandList = commandQueue->CommandList();

		auto& backBuffer = m_backBufferTextures[m_currentBackBuffer];
		if (texture.IsValid()) {
			if (texture.ResourceDesc().SampleDesc.Count > 1) {
				commandList->ResolveSubresource(backBuffer, texture);
			} else {
				commandList->CopyResource(backBuffer, texture);
			}
		}

		RenderTarget target;
		target.AttachTexture(AttachmentPoint::COLOR_0, backBuffer);

		commandList->TransitionBarrier(backBuffer, D3D12_RESOURCE_STATE_PRESENT);
		commandQueue->ExecuteCommandList(commandList);

		unsigned int syncInterval = m_useVSync ? 1 : 0;
		unsigned int presentFlags = m_context.IsTearingSupported() && !m_useVSync ? DXGI_PRESENT_ALLOW_TEARING : 0;
		ThrowOnFailure(m_swapchain->Present(syncInterval, presentFlags));

		m_frameFenceValues[m_currentBackBuffer] = commandQueue->Signal();
		m_frameFenceValues[m_currentBackBuffer] = FPSCounter::FrameCount();

		m_currentBackBuffer = m_swapchain->GetCurrentBackBufferIndex();
		commandQueue->WaitForFenceValue(m_frameFenceValues[m_currentBackBuffer]);

		ReleaseStaleDescriptors(m_frameFenceValues[m_currentBackBuffer]);
		return m_currentBackBuffer;
	}

	void Application::Flush() {
		m_context.Flush();
	}

	DescriptorAllocation Application::AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors) {
		return m_context.Allocators()[type]->Allocate(numDescriptors);
	}

	void Application::ReleaseStaleDescriptors(uint64_t finishedFrame) {
		auto allocators = m_context.Allocators();
		for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++) {
			allocators[i]->ReleaseStaleDescriptors(finishedFrame);
		}
	}

	DXGI_SAMPLE_DESC Application::GetMultisampleQualityLevels(DXGI_FORMAT format, UINT numSamples, D3D12_MULTISAMPLE_QUALITY_LEVEL_FLAGS flags) const {
		DXGI_SAMPLE_DESC sampleDesc = { 1, 0 };

		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS qualityLevels;
		qualityLevels.Format = format;
		qualityLevels.SampleCount = 1;
		qualityLevels.Flags = flags;
		qualityLevels.NumQualityLevels = 0;

		while (qualityLevels.SampleCount <= numSamples && SUCCEEDED(Device()->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &qualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS))) && qualityLevels.NumQualityLevels > 0) {
			// That works...
			sampleDesc.Count = qualityLevels.SampleCount;
			sampleDesc.Quality = qualityLevels.NumQualityLevels - 1;

			// But can we do better?
			qualityLevels.SampleCount *= 2;
		}

		return sampleDesc;
	}

	void Application::TransitionResource(ComPtr<ID3D12GraphicsCommandList2> commandList, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState) {
		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			resource.Get(), beforeState, afterState
		);
		commandList->ResourceBarrier(1, &barrier);
	}

	void Application::UpdateRenderTargetViews() {
		for (int i = 0; i < m_nFrames; i++) {
			ComPtr<ID3D12Resource> backBuffer;
			ThrowOnFailure(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&backBuffer)));
		
			ResourceStateTracker::AddGlobalResourceState(backBuffer.Get(), D3D12_RESOURCE_STATE_COMMON);

			m_backBufferTextures[i].SetResource(backBuffer);
			m_backBufferTextures[i].CreateViews();
		}
	}

	ComPtr<IDXGISwapChain4> Application::CreateSwapChain(int width, int height, HWND windowHandle) {
		ComPtr<IDXGISwapChain4> dxgiSwapChain4;
		ComPtr<IDXGIFactory4> dxgiFactory4;
		UINT createFactoryFlags = 0;
#if defined(_DEBUG)
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif
			
		ThrowOnFailure(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.Width = width;
		swapChainDesc.Height = height;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.Stereo = FALSE;
		swapChainDesc.SampleDesc = { 1, 0 };
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = m_nFrames;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		// It is recommended to always allow tearing if tearing support is available.
		swapChainDesc.Flags = m_context.IsTearingSupported() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
		ID3D12CommandQueue* pCommandQueue = m_context.GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->D3D12CommandQueue().Get();

		ComPtr<IDXGISwapChain1> swapChain1;
		ThrowOnFailure(dxgiFactory4->CreateSwapChainForHwnd(
			pCommandQueue,
			windowHandle,
			&swapChainDesc,
			nullptr,
			nullptr,
			&swapChain1)
		);

		ThrowOnFailure(swapChain1.As(&dxgiSwapChain4));
		m_currentBackBuffer = dxgiSwapChain4->GetCurrentBackBufferIndex();
		return dxgiSwapChain4;
	}
}
