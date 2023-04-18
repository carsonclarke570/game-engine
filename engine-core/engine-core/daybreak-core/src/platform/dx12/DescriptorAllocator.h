#pragma once

#include "DescriptorAllocation.h"

namespace dx12 {

	class DescriptorAllocatorPage;

	class DescriptorAllocator {
		public:
			DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t nDescriptorsPerHeap = 256);
			virtual ~DescriptorAllocator();

			DescriptorAllocation Allocate(uint32_t nDescriptors = 1);
			void ReleaseStaleDescriptors(uint64_t frameNumber);

		private:
			using DescriptorHeapPool = std::vector<std::shared_ptr<DescriptorAllocatorPage>>;

			D3D12_DESCRIPTOR_HEAP_TYPE	m_heapType;
			uint32_t					m_nDescriptorsPerHeap;
			DescriptorHeapPool			m_heapPool;
			std::set<size_t>			m_availableHeaps;
			std::mutex					m_allocationMutex;

			std::shared_ptr<DescriptorAllocatorPage> CreateAllocatorPage();
	};
}