#include "daybreak.h"

#include "DescriptorAllocatorPage.h"

namespace dx12 {
	DescriptorAllocatorPage::DescriptorAllocatorPage(D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t nDescriptors)
		: m_heapType(type),
		m_nDescriptorsInHeap(nDescriptors) {

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Type = m_heapType;
		heapDesc.NumDescriptors = m_nDescriptorsInHeap;

        auto device = Application::Device();
		ThrowOnFailure(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_descriptorHeap)));

		m_baseDescriptor = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		m_descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(m_heapType);
		m_nFreeHandles = m_nDescriptorsInHeap;

		AddNewBlock(0, m_nFreeHandles);
	}
	
	bool DescriptorAllocatorPage::HasSpace(uint32_t nDescriptors) const {
		return m_freeListBySize.lower_bound(nDescriptors) != m_freeListBySize.end();
	}

	DescriptorAllocation DescriptorAllocatorPage::Allocate(uint32_t nDescriptors) {
        std::lock_guard<std::mutex> lock(m_allocationMutex);

        if (nDescriptors > m_nFreeHandles) {
            return DescriptorAllocation();
        }

        auto smallestBlockIt = m_freeListBySize.lower_bound(nDescriptors);
        if (smallestBlockIt == m_freeListBySize.end()) {
            return DescriptorAllocation();
        }

        auto blockSize = smallestBlockIt->first;
        auto offsetIt = smallestBlockIt->second;
        auto offset = offsetIt->first;

        m_freeListBySize.erase(smallestBlockIt);
        m_freeListByOffset.erase(offsetIt);

        auto newOffset = offset + nDescriptors;
        auto newSize = blockSize - nDescriptors;

        if (newSize > 0) {
            AddNewBlock(newOffset, newSize);
        }
        m_nFreeHandles -= nDescriptors;

        return DescriptorAllocation(
            CD3DX12_CPU_DESCRIPTOR_HANDLE(m_baseDescriptor, offset, m_descriptorHandleIncrementSize),
            nDescriptors, 
			m_descriptorHandleIncrementSize, 
			shared_from_this()
		);
	}

	void DescriptorAllocatorPage::Free(DescriptorAllocation&& descriptorHandle, uint64_t frameNumber) {
		auto offset = ComputeOffset(descriptorHandle.GetDescriptorHandle());
		std::lock_guard<std::mutex> lock(m_allocationMutex);
		m_staleDescriptors.emplace(offset, descriptorHandle.NumHandles(), frameNumber);
	}
	
	void DescriptorAllocatorPage::ReleaseStaleDescriptors(uint64_t frameNumber) {
        std::lock_guard<std::mutex> lock(m_allocationMutex);
        while (!m_staleDescriptors.empty() && m_staleDescriptors.front().FrameNumber <= frameNumber) {
            auto& staleDescriptor = m_staleDescriptors.front();
            auto offset = staleDescriptor.Offset;
            auto numDescriptors = staleDescriptor.Size;

            FreeBlock(offset, numDescriptors);
            m_staleDescriptors.pop();
        }
	}

	uint32_t DescriptorAllocatorPage::ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle) {
		return static_cast<uint32_t>(handle.ptr - m_baseDescriptor.ptr) / m_descriptorHandleIncrementSize;
	}
	
	void DescriptorAllocatorPage::AddNewBlock(uint32_t offset, uint32_t nDescriptors) {
        auto offsetIt = m_freeListByOffset.emplace(offset, nDescriptors);
        auto sizeIt = m_freeListBySize.emplace(nDescriptors, offsetIt.first);
        offsetIt.first->second.FreeListBySizeIter = sizeIt;
	}
	
	void DescriptorAllocatorPage::FreeBlock(uint32_t offset, uint32_t nDescriptors) {
        // Find the first element whose offset is greater than the specified offset.
        // This is the block that should appear after the block that is being freed.
        auto nextBlockIt = m_freeListByOffset.upper_bound(offset);

        // Find the block that appears before the block being freed.
        auto prevBlockIt = nextBlockIt;
        // If it's not the first block in the list.
        if (prevBlockIt != m_freeListByOffset.begin())
        {
            // Go to the previous block in the list.
            --prevBlockIt;
        }
        else
        {
            // Otherwise, just set it to the end of the list to indicate that no
            // block comes before the one being freed.
            prevBlockIt = m_freeListByOffset.end();
        }

        // Add the number of free handles back to the heap.
        // This needs to be done before merging any blocks since merging
        // blocks modifies the numDescriptors variable.
        m_nFreeHandles += nDescriptors;

        if (prevBlockIt != m_freeListByOffset.end() &&
            offset == prevBlockIt->first + prevBlockIt->second.Size)
        {
            // The previous block is exactly behind the block that is to be freed.
            //
            // PrevBlock.Offset           Offset
            // |                          |
            // |<-----PrevBlock.Size----->|<------Size-------->|
            //

            // Increase the block size by the size of merging with the previous block.
            offset = prevBlockIt->first;
            nDescriptors += prevBlockIt->second.Size;

            // Remove the previous block from the free list.
            m_freeListBySize.erase(prevBlockIt->second.FreeListBySizeIter);
            m_freeListByOffset.erase(prevBlockIt);
        }

        if (nextBlockIt != m_freeListByOffset.end() &&
            offset + nDescriptors == nextBlockIt->first)
        {
            // The next block is exactly in front of the block that is to be freed.
            //
            // Offset               NextBlock.Offset 
            // |                    |
            // |<------Size-------->|<-----NextBlock.Size----->|

            // Increase the block size by the size of merging with the next block.
            nDescriptors += nextBlockIt->second.Size;

            // Remove the next block from the free list.
            m_freeListBySize.erase(nextBlockIt->second.FreeListBySizeIter);
            m_freeListByOffset.erase(nextBlockIt);
        }

        // Add the freed block to the free list.
        AddNewBlock(offset, nDescriptors);
	}
}