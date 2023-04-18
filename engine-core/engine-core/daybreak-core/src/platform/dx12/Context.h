#pragma once

namespace dx12 {

	class DescriptorAllocator;

	class DAYBREAK_API Context {

		public:
			Context();
			~Context();

			void Create();
			void Flush();
			ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(UINT numDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE type);

			bool IsTearingSupported() const { return m_tearingSupported; }
			ComPtr<IDXGIAdapter4> Adapter() { return m_adapter; }
			ComPtr<ID3D12Device2> Device() { return m_device; }
			std::unique_ptr<DescriptorAllocator>* Allocators() { return m_descriptorAllocators; }
			std::shared_ptr<CommandQueue> GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const;

		private:
			ComPtr<IDXGIAdapter4>					m_adapter;
			ComPtr<ID3D12Device2>					m_device;
			std::shared_ptr<CommandQueue>			m_directQueue;
			std::shared_ptr<CommandQueue>			m_computeQueue;
			std::shared_ptr<CommandQueue>			m_copyQueue;
			std::unique_ptr<DescriptorAllocator>	m_descriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
			bool									m_tearingSupported;

			ComPtr<IDXGIAdapter4> CreateAdapter(bool useWARP);
			ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> adapter);

			bool TearingSupportAvailable();
	};
}