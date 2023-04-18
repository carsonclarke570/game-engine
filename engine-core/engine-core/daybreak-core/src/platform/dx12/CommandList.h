#pragma once

#include "graphics/TextureType.h"

namespace dx12 {

    class DynamicDescriptorHeap;
    class Resource;
    class ResourceStateTracker;
    class RootSignature;
    class RenderTarget;
    class Texture;
    class UploadBuffer;
    class VertexBuffer;
    class IndexBuffer;
    class Buffer;

    class DAYBREAK_API CommandList {
        public:
            CommandList(D3D12_COMMAND_LIST_TYPE type);
            virtual ~CommandList();

            void TransitionBarrier(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, bool flushBarriers = false);
            void UAVBarrier(const Resource& resource, bool flushBarriers = false);
            void AliasingBarrier(const Resource& beforeResource, const Resource& afterResource, bool flushBarriers = false);
            void FlushResourceBarriers();

            void CopyResource(Resource& dstRes, const Resource& srcRes);
            void ResolveSubresource(Resource& dstRes, const Resource& srcRes, uint32_t dstSubresource = 0, uint32_t srcSubresource = 0);

            void CopyVertexBuffer(VertexBuffer& vertexBuffer, size_t numVertices, size_t vertexStride, const void* vertexBufferData);
            template<typename T>
            void CopyVertexBuffer(VertexBuffer& vertexBuffer, const std::vector<T>& vertexBufferData) {
                CopyVertexBuffer(vertexBuffer, vertexBufferData.size(), sizeof(T), vertexBufferData.data());
            }

            void CopyIndexBuffer(IndexBuffer& indexBuffer, size_t numIndicies, DXGI_FORMAT indexFormat, const void* indexBufferData);
            template<typename T>
            void CopyIndexBuffer(IndexBuffer& indexBuffer, const std::vector<T>& indexBufferData) {
                assert(sizeof(T) == 2 || sizeof(T) == 4);
                DXGI_FORMAT indexFormat = (sizeof(T) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
                CopyIndexBuffer(indexBuffer, indexBufferData.size(), indexFormat, indexBufferData.data());
            }

            void SetVertexBuffer(uint32_t slot, const VertexBuffer& vertexBuffer);
            void SetDynamicVertexBuffer(uint32_t slot, size_t numVertices, size_t vertexSize, const void* vertexBufferData);
            template<typename T>
            void SetDynamicVertexBuffer(uint32_t slot, const std::vector<T>& vertexBufferData) {
                SetDynamicVertexBuffer(slot, vertexBufferData.size(), sizeof(T), vertexBufferData.data());
            }

            void SetIndexBuffer(const IndexBuffer& indexBuffer);
            void SetDynamicIndexBuffer(size_t numIndicies, DXGI_FORMAT indexFormat, const void* indexBufferData);
            template<typename T>
            void SetDynamicIndexBuffer(const std::vector<T>& indexBufferData) {
                static_assert(sizeof(T) == 2 || sizeof(T) == 4);
                DXGI_FORMAT indexFormat = (sizeof(T) == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
                SetDynamicIndexBuffer(indexBufferData.size(), indexFormat, indexBufferData.data());
            }

            void SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY primitiveTopology);
            void LoadTextureFromFile(Texture& texture, const std::wstring& fileName, gfx::TextureType textureUsage = gfx::TextureType::ALBEDO);
            void ClearTexture(const Texture& texture, const float clearColor[4]);
            void ClearDepthStencilTexture(const Texture& texture, D3D12_CLEAR_FLAGS clearFlags, float depth = 1.0f, uint8_t stencil = 0);
            void GenerateMips(Texture& texture);
            void CopyTextureSubresource(Texture& texture, uint32_t firstSubresource, uint32_t numSubresources, D3D12_SUBRESOURCE_DATA* subresourceData);

            void SetGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, size_t sizeInBytes, const void* bufferData);
            template<typename T>
            void SetGraphicsDynamicConstantBuffer(uint32_t rootParameterIndex, const T& data)
            {
                SetGraphicsDynamicConstantBuffer(rootParameterIndex, sizeof(T), &data);
            }

            void SetViewport(const D3D12_VIEWPORT& viewport);
            void SetViewports(const std::vector<D3D12_VIEWPORT>& viewports);

            void SetScissorRect(const D3D12_RECT& scissorRect);
            void SetScissorRects(const std::vector<D3D12_RECT>& scissorRects);

            void SetPipelineState(ComPtr<ID3D12PipelineState> pipelineState);
            void SetRenderTarget(const RenderTarget& renderTarget);

            void SetGraphicsRootSignature(const RootSignature& rootSignature);
            void SetComputeRootSignature(const RootSignature& rootSignature);

            void SetShaderResourceView(
                uint32_t rootParameterIndex,
                uint32_t descriptorOffset,
                const Resource& resource,
                D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
                D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
                UINT firstSubresource = 0,
                UINT numSubresources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                const D3D12_SHADER_RESOURCE_VIEW_DESC* srv = nullptr
            );
            void SetUnorderedAccessView(
                uint32_t rootParameterIndex,
                uint32_t descrptorOffset,
                const Resource& resource,
                D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                UINT firstSubresource = 0,
                UINT numSubresources = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
                const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav = nullptr
            );

            void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t startVertex = 0, uint32_t startInstance = 0);
            void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t startIndex = 0, int32_t baseVertex = 0, uint32_t startInstance = 0);
            void Dispatch(uint32_t numGroupsX, uint32_t numGroupsY = 1, uint32_t numGroupsZ = 1);


            // Internal only
            bool Close(CommandList& pendingCommandList);
            void Close();

            /**
             * Reset the command list. This should only be called by the CommandQueue
             * before the command list is returned from CommandQueue::GetCommandList.
             */
            void Reset();

            /**
             * Release tracked objects. Useful if the swap chain needs to be resized.
             */
            void ReleaseTrackedObjects();

            /**
             * Set the currently bound descriptor heap.
             * Should only be called by the DynamicDescriptorHeap class.
             */
            void SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, ID3D12DescriptorHeap* heap);


            D3D12_COMMAND_LIST_TYPE CommandListType() const { return m_type; }
            ComPtr<ID3D12GraphicsCommandList2> GraphicsCommandList() const { return m_list; }

        private:
            void TrackObject(ComPtr<ID3D12Object> object);
            void TrackResource(const Resource& res);
            void BindDescriptorHeaps();

            void GenerateMips_UAV(Texture& texture);
            void GenerateMips_BGR(Texture& texture);
            void GenerateMips_sRGB(Texture& texture);

            void CopyBuffer(Buffer& buffer, size_t numElements, size_t elementSize, const void* bufferData, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE);


            using TrackedObjects = std::vector<ComPtr<ID3D12Object>>;


            D3D12_COMMAND_LIST_TYPE             m_type;
            ComPtr<ID3D12GraphicsCommandList2>  m_list;
            ComPtr<ID3D12CommandAllocator>      m_allocator;

            std::shared_ptr<CommandList>            m_computeCommandList;
            ID3D12RootSignature*                    m_rootSignature;
            std::unique_ptr<UploadBuffer>           m_uploadBuffer;
            std::unique_ptr<ResourceStateTracker>   m_resourceStateTracker;
            std::unique_ptr<DynamicDescriptorHeap>  m_dynamicDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];
            ID3D12DescriptorHeap*                   m_descriptorHeaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

            TrackedObjects m_trackedObjects;

            static std::map<std::wstring, ID3D12Resource*>  g_textureCache;
            static std::mutex                               g_textureCacheMutex;
    };

    
}