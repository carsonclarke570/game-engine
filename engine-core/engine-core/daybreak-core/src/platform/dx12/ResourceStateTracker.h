#pragma once

namespace dx12 {

	class CommandList;
	class Resource;

	class ResourceStateTracker {
		public:
			static void Lock();
			static void Unlock();
			static void AddGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state);
			static void RemoveGlobalResourceState(ID3D12Resource* resource);

			ResourceStateTracker();
			virtual ~ResourceStateTracker();

			void ResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier);
			void TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
			void TransitionResource(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);

			void UAVBarrier(const Resource* resource = nullptr);
			void AliasBarrier(const Resource* resourceBefore = nullptr, const Resource* resourceAfter = nullptr);
	
			uint32_t FlushPendingResourceBarriers(CommandList& commandList);
			void FlushResourceBarriers(CommandList& commandList);

			void CommitFinalResourceStates();
			void Reset();

	private:
        struct ResourceState {

            explicit ResourceState(D3D12_RESOURCE_STATES state = D3D12_RESOURCE_STATE_COMMON)
                : State(state)
            {}

            void SetSubresourceState(UINT subresource, D3D12_RESOURCE_STATES state) {
                if (subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) {
                    State = state;
                    SubresourceState.clear();
                } else {
                    SubresourceState[subresource] = state;
                }
            }

            D3D12_RESOURCE_STATES GetSubresourceState(UINT subresource) const {
                D3D12_RESOURCE_STATES state = State;
                const auto iter = SubresourceState.find(subresource);
                if (iter != SubresourceState.end()) {
                    state = iter->second;
                }
                return state;
            }

            D3D12_RESOURCE_STATES					State;
            std::map<UINT, D3D12_RESOURCE_STATES>	SubresourceState;
        };

		std::vector<D3D12_RESOURCE_BARRIER>					m_pendingResourceBarriers;
		std::vector<D3D12_RESOURCE_BARRIER>					m_resourceBarriers;
		std::unordered_map<ID3D12Resource*, ResourceState>	m_finalResourceState;

		static std::unordered_map<ID3D12Resource*, ResourceState>	g_resourceState;
		static std::mutex											g_resourceStateMutex;
		static bool													g_resourceStateLocked;
	};
}