#include "daybreak.h"

#include "DescriptorAllocator.h"
#include "DescriptorAllocatorPage.h"

namespace dx12 {
	DescriptorAllocator::DescriptorAllocator(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t nDescriptorsPerHeap)
		: m_heapType(type),
		m_nDescriptorsPerHeap(nDescriptorsPerHeap) {}
	
	DescriptorAllocator::~DescriptorAllocator() {}

	DescriptorAllocation DescriptorAllocator::Allocate(uint32_t nDescriptors) {
		std::lock_guard<std::mutex> lock(m_allocationMutex);
		DescriptorAllocation allocation;

		for (auto iter = m_availableHeaps.begin(); iter != m_availableHeaps.end(); ++iter) {
			auto page = m_heapPool[*iter];
			allocation = page->Allocate(nDescriptors);

			if (page->NumFreeHandles() == 0) {
				iter = m_availableHeaps.erase(iter);
			}

			if (!allocation.IsNull()) {
				break;
			}
		}

		if (allocation.IsNull()) {
			m_nDescriptorsPerHeap = std::max(m_nDescriptorsPerHeap, nDescriptors);
			auto newPage = CreateAllocatorPage();
			allocation = newPage->Allocate(nDescriptors);
		}

		return allocation;
	}

	void DescriptorAllocator::ReleaseStaleDescriptors(uint64_t frameNumber) {
		std::lock_guard<std::mutex> lock(m_allocationMutex);

		for (size_t i = 0; i < m_heapPool.size(); ++i) {
			auto page = m_heapPool[i];
			page->ReleaseStaleDescriptors(frameNumber);

			if (page->NumFreeHandles() > 0) {
				m_availableHeaps.insert(i);
			}
		}
	}

	std::shared_ptr<DescriptorAllocatorPage> DescriptorAllocator::CreateAllocatorPage() {
		auto newPage = std::make_shared<DescriptorAllocatorPage>(m_heapType, m_nDescriptorsPerHeap);
		m_heapPool.emplace_back(newPage);
		m_availableHeaps.insert(m_heapPool.size() - 1);
		return newPage;
	}
}