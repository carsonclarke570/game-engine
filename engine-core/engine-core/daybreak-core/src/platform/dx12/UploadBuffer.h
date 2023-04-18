#pragma once

namespace dx12 {

	class DAYBREAK_API UploadBuffer {

		public:
			struct Allocation {
				void*						cpuAddress;
				D3D12_GPU_VIRTUAL_ADDRESS	gpuAddress;
			};

			UploadBuffer(size_t pageSize = MEMSIZE_4MB);
			virtual ~UploadBuffer();

			Allocation Allocate(size_t sizeBytes, size_t alignment);
			void Reset();

			size_t PageSize() const { return m_pageSize; }
	
		private:
			struct Page {
				
					Page(size_t sizeBytes);
					~Page();

					bool HasSpace(size_t sizeBytes, size_t alignment) const;
					Allocation Allocate(size_t sizeBytes, size_t alignment);

					void Reset();

				private:
					ComPtr<ID3D12Resource>		m_resource;

					// Base Pointers
					void* m_cpuBase;
					D3D12_GPU_VIRTUAL_ADDRESS	m_gpuBase;

					size_t						m_pageSize;
					size_t						m_offset;
			};

			using PagePool = std::deque<std::shared_ptr<Page>>;

			PagePool				m_pagePool;
			PagePool				m_availablePages;
			size_t					m_pageSize;
			std::shared_ptr<Page>	m_currentPage;

			std::shared_ptr<Page> RequestPage();
	};
}