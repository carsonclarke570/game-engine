#pragma once

namespace dx12 {

	class DescriptorAllocatorPage;
      
	class DAYBREAK_API DescriptorAllocation {
	public:
        DescriptorAllocation();
        DescriptorAllocation(D3D12_CPU_DESCRIPTOR_HANDLE descriptor, uint32_t nHandles, uint32_t descriptorSize, std::shared_ptr<DescriptorAllocatorPage> page);
        ~DescriptorAllocation();

        DescriptorAllocation(const DescriptorAllocation&) = delete;
        DescriptorAllocation& operator=(const DescriptorAllocation&) = delete;

        DescriptorAllocation(DescriptorAllocation&& allocation) noexcept;
        DescriptorAllocation& operator=(DescriptorAllocation&& other) noexcept;
        D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle(uint32_t offset = 0) const;

        bool IsNull() const { return m_descriptor.ptr == 0; }
        uint32_t NumHandles() const { return m_nHandles; }
        std::shared_ptr<DescriptorAllocatorPage> GetDescriptorAllocatorPage() const { return m_page; }

	private:
        void Free();

		D3D12_CPU_DESCRIPTOR_HANDLE					m_descriptor;
		uint32_t									m_nHandles;
		uint32_t									m_descriptorSize;
		std::shared_ptr<DescriptorAllocatorPage>	m_page;
	};
}