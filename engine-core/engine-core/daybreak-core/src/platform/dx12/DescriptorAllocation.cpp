#include "daybreak.h"
#include "DescriptorAllocation.h"
#include "DescriptorAllocatorPage.h"

#include "engine/manager/FPSCounter.h"

namespace dx12 {
	DescriptorAllocation::DescriptorAllocation() 
		: m_descriptor{ 0 },
		m_nHandles(0),
		m_descriptorSize(0),
		m_page(nullptr) {}
	
	DescriptorAllocation::DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, uint32_t nHandles, uint32_t descriptorSize, std::shared_ptr<DescriptorAllocatorPage> page)
		: m_descriptor(descriptor),
		m_nHandles(nHandles),
		m_descriptorSize(descriptorSize),
		m_page(page) {}
	
	DescriptorAllocation::~DescriptorAllocation() {
		Free();
	}

	DescriptorAllocation::DescriptorAllocation(DescriptorAllocation&& allocation) noexcept
		: m_descriptor(allocation.m_descriptor),
		m_nHandles(allocation.m_nHandles),
		m_descriptorSize(allocation.m_descriptorSize),
		m_page(std::move(allocation.m_page)) {

		allocation.m_descriptor.ptr = 0;
		allocation.m_nHandles = 0;
		allocation.m_descriptorSize = 0;
	}

	DescriptorAllocation& DescriptorAllocation::operator=(DescriptorAllocation&& other) noexcept {
		Free();

		m_descriptor = other.m_descriptor;
		m_nHandles = other.m_nHandles;
		m_descriptorSize = other.m_descriptorSize;
		m_page = std::move(other.m_page);

		other.m_descriptor.ptr = 0;
		other.m_nHandles = 0;
		other.m_descriptorSize = 0;

		return *this;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE DescriptorAllocation::GetDescriptorHandle(uint32_t offset) const {
		assert(offset < m_nHandles);
		return { m_descriptor.ptr + (m_descriptorSize * offset) };
	}

	void DescriptorAllocation::Free() {
		if (!IsNull() && m_page) {
			// m_page->Free(std::move(*this), FPSCounter::FrameCount());

			m_descriptor.ptr = 0;
			m_nHandles = 0;
			m_descriptorSize = 0;
			m_page.reset();
		}
	}
}