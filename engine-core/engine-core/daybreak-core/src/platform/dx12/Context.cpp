#include "daybreak.h"

#include "DescriptorAllocator.h"

namespace dx12 {
	
	Context::Context() 
		: m_tearingSupported(false), m_directQueue(), m_computeQueue(), m_copyQueue() {}

	Context::~Context() {
		Flush();
	}

	void Context::Create() {
		SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

		if (Engine::GetMode() == EngineMode::DEBUG || Engine::GetMode() == EngineMode::EDITOR) {
			Logger::info(L"[DX12Context] Enabling debug interface...\n");
			ComPtr<ID3D12Debug> debugInterface;
			ThrowOnFailure(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
			debugInterface->EnableDebugLayer();
		}

		Logger::info(L"[DX12Context] Creating adapter...\n");
		m_adapter = CreateAdapter(false);

		Logger::info(L"[DX12Context] Creating device...\n");
		m_device = CreateDevice(m_adapter);

		Logger::info(L"[DX12Context] Creating command queues...\n");
		m_directQueue = std::make_shared<CommandQueue>(D3D12_COMMAND_LIST_TYPE_DIRECT);
		m_computeQueue = std::make_shared<CommandQueue>(D3D12_COMMAND_LIST_TYPE_COMPUTE);
		m_copyQueue = std::make_shared<CommandQueue>(D3D12_COMMAND_LIST_TYPE_COPY);
		m_directQueue->Initialize(m_device);
		m_computeQueue->Initialize(m_device);
		m_copyQueue->Initialize(m_device);

		m_tearingSupported = TearingSupportAvailable();

		Logger::info(L"[Context] Creating descriptor allocators...\n");
		for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++) {
			m_descriptorAllocators[i] = std::make_unique<DescriptorAllocator>(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
		}
	}

	void Context::Flush() {
		m_directQueue->Flush();
		m_computeQueue->Flush();
		m_copyQueue->Flush();
	}

	ComPtr<ID3D12DescriptorHeap> Context::CreateDescriptorHeap(UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type) {
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = type;
		desc.NumDescriptors = numDescriptors;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
		ThrowOnFailure(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)));

		return descriptorHeap;
	}

	std::shared_ptr<CommandQueue> Context::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const {
		std::shared_ptr<CommandQueue> commandQueue;
		switch (type) {
			case D3D12_COMMAND_LIST_TYPE_DIRECT:
				commandQueue = m_directQueue;
				break;
			case D3D12_COMMAND_LIST_TYPE_COMPUTE:
				commandQueue = m_computeQueue;
				break;
			case D3D12_COMMAND_LIST_TYPE_COPY:
				commandQueue = m_copyQueue;
				break;
			default:
				assert(false && "Invalid command queue type.");
		}
		return commandQueue;
	}

	ComPtr<IDXGIAdapter4> Context::CreateAdapter(bool useWARP) {
		ComPtr<IDXGIFactory4> dxgiFactory2;

		UINT createFactoryFlags = 0;
#ifdef _DEBUG
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif 

		ThrowOnFailure(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory2)));
		ComPtr<IDXGIAdapter1> dxgiAdapter1;
		ComPtr<IDXGIAdapter4> dxgiAdapter4;

		if (useWARP) {
			ThrowOnFailure(dxgiFactory2->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
			ThrowOnFailure(dxgiAdapter1.As(&dxgiAdapter4));
		}
		else {
			size_t maxDedicatedVideoMemory = 0;
			for (int i = 0; dxgiFactory2->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; i++) {
				DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
				dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);
				if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0
					&& SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_1, __uuidof(ID3D12Device), nullptr))
					&& dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory) {
					maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
					ThrowOnFailure(dxgiAdapter1.As(&dxgiAdapter4));
				}
			}
		}
		return dxgiAdapter4;
	}

	ComPtr<ID3D12Device2> Context::CreateDevice(ComPtr<IDXGIAdapter4> adapter) {
		ComPtr<ID3D12Device2> device;
		ThrowOnFailure(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
#ifdef _DEBUG
		ComPtr<ID3D12InfoQueue> infoQueue;
		if (SUCCEEDED(device.As(&infoQueue))) {
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
			infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

			D3D12_INFO_QUEUE_FILTER filter = {};
			ThrowOnFailure(infoQueue->PushStorageFilter(&filter));
		}
#endif	
		
		return device;
	}
	bool Context::TearingSupportAvailable() {
		bool support = false;
		ComPtr<IDXGIFactory4> factory4;

		if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
		{
			ComPtr<IDXGIFactory5> factory5;
			if (SUCCEEDED(factory4.As(&factory5)))
			{
				if (FAILED(factory5->CheckFeatureSupport(
					DXGI_FEATURE_PRESENT_ALLOW_TEARING,
					&support, sizeof(support)
				))) {
					support = false;
				}
			}
		}
		return support;
	}
}




