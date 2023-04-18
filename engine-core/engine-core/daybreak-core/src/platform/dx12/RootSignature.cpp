#include "daybreak.h"

#include "RootSignature.h"

namespace dx12 {

	RootSignature::RootSignature()
		: m_desc{},
		m_numDescriptorsPerTable{0},
		m_samplerTableBitMask(0),
		m_descriptorTableBitMask(0) {}

	RootSignature::RootSignature(const D3D12_ROOT_SIGNATURE_DESC1& desc, D3D_ROOT_SIGNATURE_VERSION version) 
		: m_desc{},
		m_numDescriptorsPerTable{ 0 },
		m_samplerTableBitMask(0),
		m_descriptorTableBitMask(0) {
		SetRootSignatureDesc(desc, version);
	}

	RootSignature::~RootSignature() {
		Destroy();
	}

	void RootSignature::Destroy() {
		for (uint32_t i = 0; i < m_desc.NumParameters; ++i) {
			const D3D12_ROOT_PARAMETER1& rootParameter = m_desc.pParameters[i];
			if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
				delete[] rootParameter.DescriptorTable.pDescriptorRanges;
			}
		}

		delete[] m_desc.pParameters;
		m_desc.pParameters = nullptr;
		m_desc.NumParameters = 0;

		delete[] m_desc.pStaticSamplers;
		m_desc.pStaticSamplers = nullptr;
		m_desc.NumStaticSamplers = 0;

		m_descriptorTableBitMask = 0;
		m_samplerTableBitMask = 0;

		memset(m_numDescriptorsPerTable, 0, sizeof(m_numDescriptorsPerTable));
	}

	void RootSignature::SetRootSignatureDesc(const D3D12_ROOT_SIGNATURE_DESC1& desc, D3D_ROOT_SIGNATURE_VERSION version) {
		Destroy();
	
        auto device = Application::Device();

        UINT numParameters = desc.NumParameters;
        D3D12_ROOT_PARAMETER1* pParameters = numParameters > 0 ? new D3D12_ROOT_PARAMETER1[numParameters] : nullptr;

        for (UINT i = 0; i < numParameters; ++i) {
            const D3D12_ROOT_PARAMETER1& rootParameter = desc.pParameters[i];
            pParameters[i] = rootParameter;

            if (rootParameter.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
                UINT numDescriptorRanges = rootParameter.DescriptorTable.NumDescriptorRanges;
                D3D12_DESCRIPTOR_RANGE1* pDescriptorRanges = numDescriptorRanges > 0 ? new D3D12_DESCRIPTOR_RANGE1[numDescriptorRanges] : nullptr;

                memcpy(pDescriptorRanges, rootParameter.DescriptorTable.pDescriptorRanges, sizeof(D3D12_DESCRIPTOR_RANGE1) * numDescriptorRanges);

                pParameters[i].DescriptorTable.NumDescriptorRanges = numDescriptorRanges;
                pParameters[i].DescriptorTable.pDescriptorRanges = pDescriptorRanges;

                // Set the bit mask depending on the type of descriptor table.
                if (numDescriptorRanges > 0) {
                    switch (pDescriptorRanges[0].RangeType) {
                        case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
                        case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
                        case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
                            m_descriptorTableBitMask |= (1 << i);
                            break;
                        case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
                            m_samplerTableBitMask |= (1 << i);
                            break;
                    }
                }

                // Count the number of descriptors in the descriptor table.
                for (UINT j = 0; j < numDescriptorRanges; ++j) {
                    m_numDescriptorsPerTable[i] += pDescriptorRanges[j].NumDescriptors;
                }
            }
        }

        m_desc.NumParameters = numParameters;
        m_desc.pParameters = pParameters;

        UINT numStaticSamplers = m_desc.NumStaticSamplers;
        D3D12_STATIC_SAMPLER_DESC* pStaticSamplers = numStaticSamplers > 0 ? new D3D12_STATIC_SAMPLER_DESC[numStaticSamplers] : nullptr;

        if (pStaticSamplers) {
            memcpy(pStaticSamplers, m_desc.pStaticSamplers, sizeof(D3D12_STATIC_SAMPLER_DESC) * numStaticSamplers);
        }

		m_desc.NumStaticSamplers = numStaticSamplers;
		m_desc.pStaticSamplers = pStaticSamplers;

        D3D12_ROOT_SIGNATURE_FLAGS flags = desc.Flags;
		m_desc.Flags = flags;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC versionRootSignatureDesc;
        versionRootSignatureDesc.Init_1_1(numParameters, pParameters, numStaticSamplers, pStaticSamplers, flags);

        // Serialize the root signature.
        Microsoft::WRL::ComPtr<ID3DBlob> rootSignatureBlob;
        Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
        ThrowOnFailure(D3DX12SerializeVersionedRootSignature(&versionRootSignatureDesc, version, &rootSignatureBlob, &errorBlob));

        // Create the root signature.
		ThrowOnFailure(device->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
            rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	}
	
	uint32_t RootSignature::DescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) const {
		uint32_t descriptorTableBitMask = 0;
		switch (descriptorHeapType) {
			case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
				descriptorTableBitMask = m_descriptorTableBitMask;
				break;
			case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
				descriptorTableBitMask = m_samplerTableBitMask;
				break;
		}
		return descriptorTableBitMask;
	}

	uint32_t RootSignature::NumDescriptors(uint32_t rootIndex) const {
		assert(rootIndex < 32);
		return m_numDescriptorsPerTable[rootIndex];
	}
}


