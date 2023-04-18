#pragma once

#include "DescriptorAllocation.h"

namespace dx12 {

	class DescriptorAllocatorPage : public std::enable_shared_from_this<DescriptorAllocatorPage> {
	public:
		DescriptorAllocatorPage(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t nDescriptors);
		
		bool HasSpace(uint32_t nDescriptors) const;
		DescriptorAllocation Allocate(uint32_t nDescriptors);
		void Free(DescriptorAllocation&& descriptorHandle, uint64_t frameNumber);
		void ReleaseStaleDescriptors(uint64_t frameNumber);
		
		D3D12_DESCRIPTOR_HEAP_TYPE HeapType() const { return m_heapType; }
		uint32_t NumFreeHandles() const { return m_nFreeHandles; }

	protected:
		uint32_t ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle);
		void AddNewBlock(uint32_t offset, uint32_t nDescriptors);
		void FreeBlock(uint32_t offset, uint32_t nDescriptors);

	private:
		struct FreeBlockInfo;
		struct StaleDescriptorInfo;

		using OffsetType = uint32_t;
		using SizeType = uint32_t;
		using FreeListByOffset = std::map<OffsetType, FreeBlockInfo>;
		using FreeListBySize = std::multimap<SizeType, FreeListByOffset::iterator>;
		using StaleDescriptorQueue = std::queue<StaleDescriptorInfo>;

		struct FreeBlockInfo {
			FreeBlockInfo(SizeType size) : Size(size) {}

			SizeType					Size;
			FreeListBySize::iterator	FreeListBySizeIter;
		};

		struct StaleDescriptorInfo {
			StaleDescriptorInfo(OffsetType offset, SizeType size, uint64_t frame)
				: Offset(offset), 
				Size(size), 
				FrameNumber(frame) {}

			OffsetType	Offset;
			SizeType	Size;
			uint64_t	FrameNumber;
		};

		FreeListByOffset								m_freeListByOffset;
		FreeListBySize									m_freeListBySize;
		StaleDescriptorQueue							m_staleDescriptors;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>	m_descriptorHeap;
		D3D12_DESCRIPTOR_HEAP_TYPE						m_heapType;
		CD3DX12_CPU_DESCRIPTOR_HANDLE					m_baseDescriptor;
		uint32_t										m_descriptorHandleIncrementSize;
		uint32_t										m_nDescriptorsInHeap;
		uint32_t										m_nFreeHandles;

		std::mutex										m_allocationMutex;
	};	
}