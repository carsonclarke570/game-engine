#include "daybreak.h"
#include "UploadBuffer.h"

namespace dx12 {
	UploadBuffer::UploadBuffer(size_t pageSize) 
		: m_pageSize(pageSize) {}

	UploadBuffer::~UploadBuffer() {}

	UploadBuffer::Allocation UploadBuffer::Allocate(size_t sizeBytes, size_t alignment) {
		if (sizeBytes > m_pageSize) {
			throw std::bad_alloc();
		}

		// If there is no current page, or the requested allocation exceeds the
		// remaining space in the current page, request a new page.
		if (!m_currentPage || !m_currentPage->HasSpace(sizeBytes, alignment)) {
			m_currentPage = RequestPage();
		}
		return m_currentPage->Allocate(sizeBytes, alignment);
	}
	
	void UploadBuffer::Reset() {
		m_currentPage = nullptr;
		m_availablePages = m_pagePool;
		for (auto &page : m_availablePages) {
			page->Reset();
		}
	}

	std::shared_ptr<UploadBuffer::Page> UploadBuffer::RequestPage()
	{
		std::shared_ptr<Page> page;
		if (!m_availablePages.empty()) {
			page = m_availablePages.front();
			m_availablePages.pop_front();
		} else {
			page = std::make_shared<Page>(m_pageSize);
			m_pagePool.push_back(page);
		}
		return page;
	}

	UploadBuffer::Page::Page(size_t sizeInBytes) :
		m_pageSize(sizeInBytes), 
		m_offset(0), 
		m_cpuBase(nullptr), 
		m_gpuBase(D3D12_GPU_VIRTUAL_ADDRESS(0)) {

		auto device = Application::Device();
		auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(m_pageSize);
		ThrowOnFailure(device->CreateCommittedResource(
			&heapProperties,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_resource)
		));

		m_gpuBase = m_resource->GetGPUVirtualAddress();
		m_resource->Map(0, nullptr, &m_cpuBase);
	}

	UploadBuffer::Page::~Page() {
		m_resource->Unmap(0, nullptr);
		m_cpuBase = nullptr;
		m_gpuBase = D3D12_GPU_VIRTUAL_ADDRESS(0);
	}

	bool UploadBuffer::Page::HasSpace(size_t sizeBytes, size_t alignment) const {
		size_t alignedSize = AlignUp(sizeBytes, alignment);
		size_t alignedOffset = AlignUp(m_offset, alignment);

		return alignedOffset + alignedSize <= m_pageSize;
	}

	UploadBuffer::Allocation UploadBuffer::Page::Allocate(size_t sizeBytes, size_t alignment) {
		if (!HasSpace(sizeBytes, alignment)) {
			throw std::bad_alloc();
		}

		size_t alignedSize = AlignUp(sizeBytes, alignment);
		m_offset = AlignUp(m_offset, alignment);

		Allocation allocation;
		allocation.cpuAddress = static_cast<uint8_t*>(m_cpuBase) + m_offset;
		allocation.gpuAddress = m_gpuBase + m_offset;

		m_offset += alignedSize;

		return allocation;
	}

	void UploadBuffer::Page::Reset()
	{
		m_offset = 0;
	}
}