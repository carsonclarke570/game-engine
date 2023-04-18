#pragma once

namespace dx12 {

	class DAYBREAK_API RootSignature {
		public:
			RootSignature();
			RootSignature(const D3D12_ROOT_SIGNATURE_DESC1& desc, D3D_ROOT_SIGNATURE_VERSION version);
			virtual ~RootSignature();

			void Destroy();
			void SetRootSignatureDesc(const D3D12_ROOT_SIGNATURE_DESC1& desc, D3D_ROOT_SIGNATURE_VERSION version);

			uint32_t DescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) const;
			uint32_t NumDescriptors(uint32_t rootIndex) const;
			ComPtr<ID3D12RootSignature> Signature() const { return m_rootSignature; }
			const D3D12_ROOT_SIGNATURE_DESC1& Desc() const { return m_desc; }

		private:
			D3D12_ROOT_SIGNATURE_DESC1	m_desc;
			ComPtr<ID3D12RootSignature>	m_rootSignature;

			uint32_t					m_numDescriptorsPerTable[32];
			uint32_t					m_samplerTableBitMask;
			uint32_t					m_descriptorTableBitMask;
	};

}