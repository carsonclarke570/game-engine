#include "daybreak.h"

#include "CommandList.h"

#include "DynamicDescriptorHeap.h"
#include "Resource.h"
#include "ResourceStateTracker.h"
#include "RootSignature.h"
#include "RenderTarget.h"
#include "UploadBuffer.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace dx12 {

	std::map<std::wstring, ID3D12Resource*>	CommandList::g_textureCache;
	std::mutex								CommandList::g_textureCacheMutex;

	CommandList::CommandList(D3D12_COMMAND_LIST_TYPE type) :
		m_type(type) {
	
		auto device = Application::Device();
		ThrowOnFailure(device->CreateCommandAllocator(m_type, IID_PPV_ARGS(&m_allocator)));
		ThrowOnFailure(device->CreateCommandList(0, m_type, m_allocator.Get(), nullptr, IID_PPV_ARGS(&m_list)));

		m_uploadBuffer = std::make_unique<UploadBuffer>();
		m_resourceStateTracker = std::make_unique<ResourceStateTracker>();
		for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
			m_dynamicDescriptorHeap[i] = std::make_unique<DynamicDescriptorHeap>(static_cast<D3D12_DESCRIPTOR_HEAP_TYPE>(i));
			m_descriptorHeaps[i] = nullptr;
		}
	}

	CommandList::~CommandList() { }

	void CommandList::TransitionBarrier(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource, bool flushBarriers) {
		auto res = resource.Get();
		if (res) {
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(res.Get(), D3D12_RESOURCE_STATE_COMMON, stateAfter, subresource);
			m_resourceStateTracker->ResourceBarrier(barrier);
		}

		if (flushBarriers) {
			FlushResourceBarriers();
		}
	}
	
	void CommandList::UAVBarrier(const Resource& resource, bool flushBarriers) {
		auto res = resource.Get();
		auto barrier = CD3DX12_RESOURCE_BARRIER::UAV(res.Get());

		m_resourceStateTracker->ResourceBarrier(barrier);
		if (flushBarriers) {
			FlushResourceBarriers();
		}
	}

	void CommandList::AliasingBarrier(const Resource& beforeResource, const Resource& afterResource, bool flushBarriers) {
		auto d3d12BeforeResource = beforeResource.Get();
		auto d3d12AfterResource = afterResource.Get();
		auto barrier = CD3DX12_RESOURCE_BARRIER::Aliasing(d3d12BeforeResource.Get(), d3d12AfterResource.Get());

		m_resourceStateTracker->ResourceBarrier(barrier);
		if (flushBarriers) {
			FlushResourceBarriers();
		}
	}

	void CommandList::FlushResourceBarriers() {
		m_resourceStateTracker->FlushResourceBarriers(*this);
	}

	void CommandList::CopyResource(Resource& dstRes, const Resource& srcRes) {
		TransitionBarrier(dstRes, D3D12_RESOURCE_STATE_COPY_DEST);
		TransitionBarrier(srcRes, D3D12_RESOURCE_STATE_COPY_SOURCE);

		FlushResourceBarriers();

		m_list->CopyResource(dstRes.Get().Get(), srcRes.Get().Get());

		TrackResource(dstRes);
		TrackResource(srcRes);
	}

	void CommandList::ResolveSubresource(Resource& dstRes, const Resource& srcRes, uint32_t dstSubresource, uint32_t srcSubresource) {
		TransitionBarrier(dstRes, D3D12_RESOURCE_STATE_RESOLVE_DEST, dstSubresource);
		TransitionBarrier(srcRes, D3D12_RESOURCE_STATE_RESOLVE_SOURCE, srcSubresource);

		FlushResourceBarriers();

		m_list->ResolveSubresource(dstRes.Get().Get(), dstSubresource, srcRes.Get().Get(), srcSubresource, dstRes.ResourceDesc().Format);

		TrackResource(srcRes);
		TrackResource(dstRes);
	}

	void CommandList::CopyVertexBuffer(VertexBuffer& vertexBuffer, size_t numVertices, size_t vertexStride, const void* vertexBufferData) {
		CopyBuffer(vertexBuffer, numVertices, vertexStride, vertexBufferData);
	}

	void CommandList::CopyIndexBuffer(IndexBuffer& indexBuffer, size_t numIndicies, DXGI_FORMAT indexFormat, const void* indexBufferData) {
		size_t indexSizeInBytes = indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4;
		CopyBuffer(indexBuffer, numIndicies, indexSizeInBytes, indexBufferData);
	}

	void CommandList::SetVertexBuffer(uint32_t slot, const VertexBuffer& vertexBuffer) {
		TransitionBarrier(vertexBuffer, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

		auto vertexBufferView = vertexBuffer.VertexBufferView();
		m_list->IASetVertexBuffers(slot, 1, &vertexBufferView);

		TrackResource(vertexBuffer);
	}

	void CommandList::SetDynamicVertexBuffer(uint32_t slot, size_t numVertices, size_t vertexSize, const void* vertexBufferData) {
		size_t bufferSize = numVertices * vertexSize;

		auto heapAllocation = m_uploadBuffer->Allocate(bufferSize, vertexSize);
		memcpy(heapAllocation.cpuAddress, vertexBufferData, bufferSize);

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};
		vertexBufferView.BufferLocation = heapAllocation.gpuAddress;
		vertexBufferView.SizeInBytes = static_cast<UINT>(bufferSize);
		vertexBufferView.StrideInBytes = static_cast<UINT>(vertexSize);

		m_list->IASetVertexBuffers(slot, 1, &vertexBufferView);
	}

	void CommandList::SetIndexBuffer(const IndexBuffer& indexBuffer) {
		TransitionBarrier(indexBuffer, D3D12_RESOURCE_STATE_INDEX_BUFFER);

		auto indexBufferView = indexBuffer.IndexBufferView();
		m_list->IASetIndexBuffer(&indexBufferView);

		TrackResource(indexBuffer);
	}

	void CommandList::SetDynamicIndexBuffer(size_t numIndicies, DXGI_FORMAT indexFormat, const void* indexBufferData) {
		size_t indexSizeInBytes = indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4;
		size_t bufferSize = numIndicies * indexSizeInBytes;

		auto heapAllocation = m_uploadBuffer->Allocate(bufferSize, indexSizeInBytes);
		memcpy(heapAllocation.cpuAddress, indexBufferData, bufferSize);

		D3D12_INDEX_BUFFER_VIEW indexBufferView = {};
		indexBufferView.BufferLocation = heapAllocation.gpuAddress;
		indexBufferView.SizeInBytes = static_cast<UINT>(bufferSize);
		indexBufferView.Format = indexFormat;

		m_list->IASetIndexBuffer(&indexBufferView);
	}

	void CommandList::SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology) {
		m_list->IASetPrimitiveTopology(primitiveTopology);
	}

	void CommandList::LoadTextureFromFile(Texture& texture, const std::wstring& fileName, gfx::TextureType textureUsage) {
		auto device = Application::Device();
		std::filesystem::path filePath(fileName);
		if (!std::filesystem::exists(filePath)) {
			throw std::exception("File not found");
		}

		auto iter = g_textureCache.find(fileName);
		if (iter != g_textureCache.end()) {
			texture.SetType(textureUsage);
			texture.SetResource(iter->second);
			texture.CreateViews();
			texture.SetName(fileName);
		} else {
			TexMetadata metadata;
			ScratchImage scratchImage;
			ComPtr<ID3D12Resource> textureResource;

			if (filePath.extension() == ".dds") {
				ThrowOnFailure(LoadFromDDSFile(fileName.c_str(), DDS_FLAGS_NONE, &metadata, scratchImage));
			} else if (filePath.extension() == ".hdr") {
				ThrowOnFailure(LoadFromHDRFile(fileName.c_str(), &metadata, scratchImage));
			} else if (filePath.extension() == ".tga") {
				ThrowOnFailure(LoadFromTGAFile(fileName.c_str(), &metadata, scratchImage));
			} else {
				ThrowOnFailure(LoadFromWICFile(fileName.c_str(), WIC_FLAGS_NONE, &metadata, scratchImage));
			}

			if (textureUsage == gfx::TextureType::ALBEDO) {
				metadata.format = MakeSRGB(metadata.format);
			}

			D3D12_RESOURCE_DESC textureDesc = {};
			switch (metadata.dimension) {
				case TEX_DIMENSION_TEXTURE1D:
					textureDesc = CD3DX12_RESOURCE_DESC::Tex1D(metadata.format, static_cast<UINT64>(metadata.width), static_cast<UINT16>(metadata.arraySize));
					break;
				case TEX_DIMENSION_TEXTURE2D:
					textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(metadata.format, static_cast<UINT64>(metadata.width), static_cast<UINT>(metadata.height), static_cast<UINT16>(metadata.arraySize));
					break;
				case TEX_DIMENSION_TEXTURE3D:
					textureDesc = CD3DX12_RESOURCE_DESC::Tex3D(metadata.format, static_cast<UINT64>(metadata.width), static_cast<UINT>(metadata.height), static_cast<UINT16>(metadata.depth));
					break;
				default:
					throw std::exception("Invalid texture dimension.");
					break;
			}

			auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			ThrowOnFailure(
				device->CreateCommittedResource(&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&textureDesc,
				D3D12_RESOURCE_STATE_COMMON,
				nullptr,
				IID_PPV_ARGS(&textureResource))
			);

			// Update the global state tracker.
			ResourceStateTracker::AddGlobalResourceState(textureResource.Get(), D3D12_RESOURCE_STATE_COMMON);

			texture.SetType(textureUsage);
			texture.SetResource(textureResource);
			texture.CreateViews();
			texture.SetName(fileName);

			std::vector<D3D12_SUBRESOURCE_DATA> subresources(scratchImage.GetImageCount());
			const Image* pImages = scratchImage.GetImages();
			for (int i = 0; i < scratchImage.GetImageCount(); ++i) {
				auto& subresource = subresources[i];
				subresource.RowPitch = pImages[i].rowPitch;
				subresource.SlicePitch = pImages[i].slicePitch;
				subresource.pData = pImages[i].pixels;
			}

			CopyTextureSubresource(texture, 0, static_cast<uint32_t>(subresources.size()), subresources.data());
			if (subresources.size() < textureResource->GetDesc().MipLevels) {
				GenerateMips(texture);
			}

			// Add the texture resource to the texture cache.
			std::lock_guard<std::mutex> lock(g_textureCacheMutex);
			g_textureCache[fileName] = textureResource.Get();
		}
	}

	void CommandList::ClearTexture(const Texture& texture, const float clearColor[4]) {
		TransitionBarrier(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_list->ClearRenderTargetView(texture.GetRenderTargetView(), clearColor, 0, nullptr);
		TrackResource(texture);
	}

	void CommandList::ClearDepthStencilTexture(const Texture& texture, D3D12_CLEAR_FLAGS clearFlags, float depth, uint8_t stencil) {
		TransitionBarrier(texture, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		m_list->ClearDepthStencilView(texture.GetDepthStencilView(), clearFlags, depth, stencil, 0, nullptr);
		TrackResource(texture);
	}

	void CommandList::GenerateMips(Texture& texture) {
		if (m_type == D3D12_COMMAND_LIST_TYPE_COPY) {
			if (!m_computeCommandList) {
				m_computeCommandList = Application::Get()->CommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)->CommandList();
			}
			m_computeCommandList->GenerateMips(texture);
			return;
		}

		auto d3d12Resource = texture.Get();

		// If the texture doesn't have a valid resource, do nothing.
		if (!d3d12Resource) return;
		auto d3d12ResourceDesc = d3d12Resource->GetDesc();

		// If the texture only has a single mip level (level 0)
		// do nothing.
		if (d3d12ResourceDesc.MipLevels == 1) return;

		// Currently, only 2D textures are supported.
		if (d3d12ResourceDesc.Dimension != D3D12_RESOURCE_DIMENSION_TEXTURE2D || d3d12ResourceDesc.DepthOrArraySize != 1) {
			throw std::exception("Generate Mips only supports 2D Textures.");
		}

		if (Texture::IsUAVCompatibleFormat(d3d12ResourceDesc.Format)) {
			GenerateMips_UAV(texture);
		} else if (Texture::IsBGRFormat(d3d12ResourceDesc.Format)) {
			GenerateMips_BGR(texture);
		} else if (Texture::IsSRGBFormat(d3d12ResourceDesc.Format)) {
			GenerateMips_sRGB(texture);
		} else {
			throw std::exception("Unsupported texture format for mipmap generation.");
		}
	}

	void CommandList::CopyTextureSubresource(Texture& texture, uint32_t firstSubresource, uint32_t numSubresources, D3D12_SUBRESOURCE_DATA* subresourceData) {
		auto device = Application::Device();
		auto destinationResource = texture.Get();
		if (destinationResource) {
			// Resource must be in the copy-destination state.
			TransitionBarrier(texture, D3D12_RESOURCE_STATE_COPY_DEST);
			FlushResourceBarriers();

			UINT64 requiredSize = GetRequiredIntermediateSize(destinationResource.Get(), firstSubresource, numSubresources);

			// Create a temporary (intermediate) resource for uploading the subresources
			ComPtr<ID3D12Resource> intermediateResource;
			auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(requiredSize);
			ThrowOnFailure(device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&resDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&intermediateResource)
			));

			UpdateSubresources(m_list.Get(), destinationResource.Get(), intermediateResource.Get(), 0, firstSubresource, numSubresources, subresourceData);

			TrackObject(intermediateResource);
			TrackObject(destinationResource);
		}
	}

	void CommandList::SetGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, size_t sizeInBytes, const void* bufferData) {
		auto heapAllococation = m_uploadBuffer->Allocate(sizeInBytes, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		memcpy(heapAllococation.cpuAddress, bufferData, sizeInBytes);
		m_list->SetGraphicsRootConstantBufferView(rootParameterIndex, heapAllococation.gpuAddress);
	}

	void CommandList::SetViewport(const D3D12_VIEWPORT& viewport) {
		SetViewports({ viewport });
	}

	void CommandList::SetViewports(const std::vector<D3D12_VIEWPORT>& viewports) {
		assert(viewports.size() < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
		m_list->RSSetViewports(static_cast<UINT>(viewports.size()), viewports.data());
	}

	void CommandList::SetScissorRect(const D3D12_RECT& scissorRect) {
		SetScissorRects({ scissorRect });
	}

	void CommandList::SetScissorRects(const std::vector<D3D12_RECT>& scissorRects) {
		assert(scissorRects.size() < D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
		m_list->RSSetScissorRects(static_cast<UINT>(scissorRects.size()), scissorRects.data());
	}

	void CommandList::SetPipelineState(ComPtr<ID3D12PipelineState> pipelineState) {
		m_list->SetPipelineState(pipelineState.Get());
		TrackObject(pipelineState);
	}

	void CommandList::SetRenderTarget(const RenderTarget& renderTarget) {
		std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> renderTargetDescriptors;
		renderTargetDescriptors.reserve(AttachmentPoint::NUM_ATTACHMENT_POINTS);
		const auto& textures = renderTarget.GetTextures();

		// Bind color targets (max of 8 render targets can be bound to the rendering pipeline.
		for (int i = 0; i < 8; ++i) {
			auto& texture = textures[i];

			if (texture.IsValid()) {
				TransitionBarrier(texture, D3D12_RESOURCE_STATE_RENDER_TARGET);
				renderTargetDescriptors.push_back(texture.GetRenderTargetView());
				TrackResource(texture);
			}
		}

		const auto& depthTexture = renderTarget.GetTexture(AttachmentPoint::DEPTH_STENCIL);
		CD3DX12_CPU_DESCRIPTOR_HANDLE depthStencilDescriptor(D3D12_DEFAULT);
		if (depthTexture.Get()) {
			TransitionBarrier(depthTexture, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			depthStencilDescriptor = depthTexture.GetDepthStencilView();
			TrackResource(depthTexture);
		}

		D3D12_CPU_DESCRIPTOR_HANDLE* pDSV = depthStencilDescriptor.ptr != 0 ? &depthStencilDescriptor : nullptr;
		m_list->OMSetRenderTargets(static_cast<UINT>(renderTargetDescriptors.size()), renderTargetDescriptors.data(), FALSE, pDSV);
	}

	void CommandList::SetGraphicsRootSignature(const RootSignature& rootSignature) {
		auto rs = rootSignature.Signature().Get();
		if (m_rootSignature != rs) {
			m_rootSignature = rs;

			for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
				m_dynamicDescriptorHeap[i]->ParseRootSignature(rootSignature);
			}

			m_list->SetGraphicsRootSignature(m_rootSignature);
			TrackObject(m_rootSignature);
		}
	}

	void CommandList::SetComputeRootSignature(const RootSignature& rootSignature) {
		auto rs = rootSignature.Signature().Get();
		if (m_rootSignature != rs) {
			m_rootSignature = rs;

			for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
				m_dynamicDescriptorHeap[i]->ParseRootSignature(rootSignature);
			}

			m_list->SetComputeRootSignature(m_rootSignature);
			TrackObject(m_rootSignature);
		}
	}

	void CommandList::SetShaderResourceView(uint32_t rootParameterIndex, uint32_t descriptorOffset, const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT firstSubresource, UINT numSubresources, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv) {
		if (numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
			for (uint32_t i = 0; i < numSubresources; ++i) {
				TransitionBarrier(resource, stateAfter, firstSubresource + i);
			}
		} else {
			TransitionBarrier(resource, stateAfter);
		}

		m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(rootParameterIndex, descriptorOffset, 1, resource.GetShaderResourceView(srv));
		TrackResource(resource);
	}
	
	void CommandList::SetUnorderedAccessView(uint32_t rootParameterIndex, uint32_t descrptorOffset, const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT firstSubresource, UINT numSubresources, const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav) {
		if (numSubresources < D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
			for (uint32_t i = 0; i < numSubresources; ++i) {
				TransitionBarrier(resource, stateAfter, firstSubresource + i);
			}
		} else {
			TransitionBarrier(resource, stateAfter);
		}

		m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(rootParameterIndex, descrptorOffset, 1, resource.GetUnorderedAccessView(uav));
		TrackResource(resource);
	}

	void CommandList::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertex, uint32_t startInstance) {
		FlushResourceBarriers();
		for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
			m_dynamicDescriptorHeap[i]->CommitStagedDescriptorsForDraw(*this);
		}
		m_list->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
	}

	void CommandList::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t startIndex, int32_t baseVertex, uint32_t startInstance) {
		FlushResourceBarriers();
		for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
			m_dynamicDescriptorHeap[i]->CommitStagedDescriptorsForDraw(*this);
		}
		m_list->DrawIndexedInstanced(indexCount, instanceCount, startIndex, baseVertex, startInstance);
	}


	void CommandList::Dispatch(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) {
		FlushResourceBarriers();
		for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
			m_dynamicDescriptorHeap[i]->CommitStagedDescriptorsForDispatch(*this);
		}
		m_list->Dispatch(numGroupsX, numGroupsY, numGroupsZ);
	}

	bool CommandList::Close(CommandList& pendingCommandList) {
		// Flush any remaining barriers.
		FlushResourceBarriers();
		m_list->Close();

		// Flush pending resource barriers.
		uint32_t numPendingBarriers = m_resourceStateTracker->FlushPendingResourceBarriers(pendingCommandList);
		// Commit the final resource state to the global state.
		m_resourceStateTracker->CommitFinalResourceStates();
		return numPendingBarriers > 0;
	}

	void CommandList::Close() {
		FlushResourceBarriers();
		m_list->Close();
	}

	void CommandList::Reset() {
		ThrowOnFailure(m_allocator->Reset());
		ThrowOnFailure(m_list->Reset(m_allocator.Get(), nullptr));

		m_resourceStateTracker->Reset();
		m_uploadBuffer->Reset();

		ReleaseTrackedObjects();
		for (int i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
			m_dynamicDescriptorHeap[i]->Reset();
			m_descriptorHeaps[i] = nullptr;
		}

		m_rootSignature = nullptr;
		m_computeCommandList = nullptr;
	}

	void CommandList::ReleaseTrackedObjects() {
		m_trackedObjects.clear();
	}

	void CommandList::SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap) {
		if(m_descriptorHeaps[heapType] != heap) {
			m_descriptorHeaps[heapType] = heap;
			BindDescriptorHeaps();
		}
	}

	void CommandList::TrackObject(ComPtr<ID3D12Object> object) {
		m_trackedObjects.push_back(object);
	}

	void CommandList::TrackResource(const Resource& res) {
		TrackObject(res.Get());
	}

	void CommandList::BindDescriptorHeaps() {
		UINT numDescriptorHeaps = 0;
		ID3D12DescriptorHeap* descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {};

		for (uint32_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; ++i) {
			ID3D12DescriptorHeap* descriptorHeap = m_descriptorHeaps[i];
			if (descriptorHeap) {
				descriptorHeaps[numDescriptorHeaps++] = descriptorHeap;
			}
		}

		m_list->SetDescriptorHeaps(numDescriptorHeaps, descriptorHeaps);
	}
	void CommandList::GenerateMips_UAV(Texture& texture) {
		//if (!m_GenerateMipsPSO){
		//	m_GenerateMipsPSO = std::make_unique<GenerateMipsPSO>();
		//}

		//auto device = Application::Get().GetDevice();

		//auto resource = texture.GetD3D12Resource();
		//auto resourceDesc = resource->GetDesc();

		//auto stagingResource = resource;
		//Texture stagingTexture(stagingResource);
		//// If the passed-in resource does not allow for UAV access
		//// then create a staging resource that is used to generate
		//// the mipmap chain.
		//if ((resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) == 0)
		//{
		//	auto stagingDesc = resourceDesc;
		//	stagingDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		//	ThrowIfFailed(device->CreateCommittedResource(
		//		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		//		D3D12_HEAP_FLAG_NONE,
		//		&stagingDesc,
		//		D3D12_RESOURCE_STATE_COPY_DEST,
		//		nullptr,
		//		IID_PPV_ARGS(&stagingResource)

		//	));

		//	ResourceStateTracker::AddGlobalResourceState(stagingResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST);

		//	stagingTexture.SetD3D12Resource(stagingResource);
		//	stagingTexture.CreateViews();
		//	stagingTexture.SetName(L"Generate Mips UAV Staging Texture");

		//	CopyResource(stagingTexture, texture);
		//}

		//m_d3d12CommandList->SetPipelineState(m_GenerateMipsPSO->GetPipelineState().Get());
		//SetComputeRootSignature(m_GenerateMipsPSO->GetRootSignature());

		//GenerateMipsCB generateMipsCB;

		//for (uint32_t srcMip = 0; srcMip < resourceDesc.MipLevels - 1u; )
		//{
		//	uint64_t srcWidth = resourceDesc.Width >> srcMip;
		//	uint32_t srcHeight = resourceDesc.Height >> srcMip;
		//	uint32_t dstWidth = static_cast<uint32_t>(srcWidth >> 1);
		//	uint32_t dstHeight = srcHeight >> 1;

		//	// Determine the compute shader to use based on the dimension of the 
		//	// source texture.
		//	// 0b00(0): Both width and height are even.
		//	// 0b01(1): Width is odd, height is even.
		//	// 0b10(2): Width is even, height is odd.
		//	// 0b11(3): Both width and height are odd.
		//	generateMipsCB.SrcDimension = (srcHeight & 1) << 1 | (srcWidth & 1);

		//	// How many mipmap levels to compute this pass (max 4 mips per pass)
		//	DWORD mipCount;

		//	// The number of times we can half the size of the texture and get
		//	// exactly a 50% reduction in size.
		//	// A 1 bit in the width or height indicates an odd dimension.
		//	// The case where either the width or the height is exactly 1 is handled
		//	// as a special case (as the dimension does not require reduction).
		//	_BitScanForward(&mipCount, (dstWidth == 1 ? dstHeight : dstWidth) |
		//		(dstHeight == 1 ? dstWidth : dstHeight));
		//	// Maximum number of mips to generate is 4.
		//	mipCount = std::min<DWORD>(4, mipCount + 1);
		//	// Clamp to total number of mips left over.
		//	mipCount = (srcMip + mipCount) > resourceDesc.MipLevels ?
		//		resourceDesc.MipLevels - srcMip : mipCount;

		//	// Dimensions should not reduce to 0.
		//	// This can happen if the width and height are not the same.
		//	dstWidth = std::max<DWORD>(1, dstWidth);
		//	dstHeight = std::max<DWORD>(1, dstHeight);

		//	generateMipsCB.SrcMipLevel = srcMip;
		//	generateMipsCB.NumMipLevels = mipCount;
		//	generateMipsCB.TexelSize.x = 1.0f / (float)dstWidth;
		//	generateMipsCB.TexelSize.y = 1.0f / (float)dstHeight;

		//	SetCompute32BitConstants(GenerateMips::GenerateMipsCB, generateMipsCB);

		//	SetShaderResourceView(GenerateMips::SrcMip, 0, stagingTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, srcMip, 1);
		//	for (uint32_t mip = 0; mip < mipCount; ++mip)
		//	{
		//		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		//		uavDesc.Format = resourceDesc.Format;
		//		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		//		uavDesc.Texture2D.MipSlice = srcMip + mip + 1;

		//		SetUnorderedAccessView(GenerateMips::OutMip, mip, stagingTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, srcMip + mip + 1, 1, &uavDesc);
		//	}
		//	// Pad any unused mip levels with a default UAV. Doing this keeps the DX12 runtime happy.
		//	if (mipCount < 4)
		//	{
		//		m_DynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]->StageDescriptors(GenerateMips::OutMip, mipCount, 4 - mipCount, m_GenerateMipsPSO->GetDefaultUAV());
		//	}

		//	Dispatch(Math::DivideByMultiple(dstWidth, 8), Math::DivideByMultiple(dstHeight, 8));

		//	UAVBarrier(stagingTexture);

		//	srcMip += mipCount;
		//}

		//// Copy back to the original texture.
		//if (stagingResource != resource)
		//{
		//	CopyResource(texture, stagingTexture);
		//}
	}

	void CommandList::GenerateMips_BGR(Texture& texture)
	{
	}
	void CommandList::GenerateMips_sRGB(Texture& texture)
	{
	}

	void CommandList::CopyBuffer(Buffer& buffer, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags) {
		auto device = Application::Device();
		size_t bufferSize = numElements * elementSize;

		ComPtr<ID3D12Resource> d3d12Resource;
		if (bufferSize == 0) {
			// This will result in a NULL resource (which may be desired to define a default null resource).
		} else {
			auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags);
			ThrowOnFailure(device->CreateCommittedResource(
				&heapProperties,
				D3D12_HEAP_FLAG_NONE,
				&resDesc,
				D3D12_RESOURCE_STATE_COMMON,
				nullptr,
				IID_PPV_ARGS(&d3d12Resource))
			);

			// Add the resource to the global resource state tracker.
			ResourceStateTracker::AddGlobalResourceState(d3d12Resource.Get(), D3D12_RESOURCE_STATE_COMMON);
			if (bufferData != nullptr) {
				// Create an upload resource to use as an intermediate buffer to copy the buffer resource 
				ComPtr<ID3D12Resource> uploadResource;
				auto heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
				auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
				ThrowOnFailure(device->CreateCommittedResource(
					&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&resDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					nullptr,
					IID_PPV_ARGS(&uploadResource)));

				D3D12_SUBRESOURCE_DATA subresourceData = {};
				subresourceData.pData = bufferData;
				subresourceData.RowPitch = bufferSize;
				subresourceData.SlicePitch = subresourceData.RowPitch;

				m_resourceStateTracker->TransitionResource(d3d12Resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
				FlushResourceBarriers();

				UpdateSubresources(m_list.Get(), d3d12Resource.Get(), uploadResource.Get(), 0, 0, 1, &subresourceData);

				// Add references to resources so they stay in scope until the command list is reset.
				TrackObject(uploadResource);
			}
			TrackObject(d3d12Resource);
		}

		buffer.SetResource(d3d12Resource);
		buffer.CreateViews(numElements, elementSize);
	}
}


