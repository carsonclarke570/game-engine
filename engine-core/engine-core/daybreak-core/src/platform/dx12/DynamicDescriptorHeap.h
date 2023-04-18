#pragma once

namespace dx12 {
	class CommandList;
	class RootSignature;

	class DynamicDescriptorHeap {

        public:
            DynamicDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptorsPerHeap = 1024);
            virtual ~DynamicDescriptorHeap();

            void StageDescriptors(uint32_t rootParameterIndex, uint32_t offset, uint32_t numDescriptors, const D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptors);

            void CommitStagedDescriptors(CommandList& commandList, std::function<void(ID3D12GraphicsCommandList*, UINT, D3D12_GPU_DESCRIPTOR_HANDLE)> setFunc);
            void CommitStagedDescriptorsForDraw(CommandList& commandList);
            void CommitStagedDescriptorsForDispatch(CommandList& commandList);

            D3D12_GPU_DESCRIPTOR_HANDLE CopyDescriptor(CommandList& comandList, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor);
            void ParseRootSignature(const RootSignature& rootSignature);

            void Reset();

	    private:
            using DescriptorHeapPool = std::queue<ComPtr<ID3D12DescriptorHeap>>;

		    static const uint32_t MaxDescriptorTables = 32;

            struct DescriptorTableCache {
                DescriptorTableCache() : 
                    NumDescriptors(0), 
                    BaseDescriptor(nullptr) {}

                void Reset() {
                    NumDescriptors = 0;
                    BaseDescriptor = nullptr;
                }
            
                uint32_t                        NumDescriptors;
                D3D12_CPU_DESCRIPTOR_HANDLE*    BaseDescriptor;
            };

            D3D12_DESCRIPTOR_HEAP_TYPE                      m_type;
            uint32_t                                        m_numDescriptorsPerHeap;
            int32_t                                         m_descriptorHandleIncrementSize;
            std::unique_ptr<D3D12_CPU_DESCRIPTOR_HANDLE[]>  m_descriptorHandleCache;
            DescriptorTableCache                            m_descriptorTableCache[MaxDescriptorTables];
            uint32_t                                        m_descriptorTableBitMask;
            uint32_t                                        m_staleDescriptorTableBitMask;

            DescriptorHeapPool                              m_descriptorHeapPool;
            DescriptorHeapPool                              m_availableDescriptorHeaps;

            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>    m_currentDescriptorHeap;
            CD3DX12_GPU_DESCRIPTOR_HANDLE                   m_currentGPUDescriptorHandle;
            CD3DX12_CPU_DESCRIPTOR_HANDLE                   m_currentCPUDescriptorHandle;

            uint32_t                                        m_numFreeHandles;

            ComPtr<ID3D12DescriptorHeap> RequestDescriptorHeap();
            ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap();
            uint32_t ComputeStaleDescriptorCount() const;
	};
}

