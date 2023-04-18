#include "daybreak.h"

#include "ResourceStateTracker.h"
#include "CommandList.h"
#include "Resource.h"

namespace dx12 {

	std::mutex ResourceStateTracker::g_resourceStateMutex;
	bool ResourceStateTracker::g_resourceStateLocked = false;
	std::unordered_map<ID3D12Resource*, ResourceStateTracker::ResourceState> ResourceStateTracker::g_resourceState;

	
	void ResourceStateTracker::Lock() {
        g_resourceStateMutex.lock();
        g_resourceStateLocked = true;
	}

	void ResourceStateTracker::Unlock() {
        g_resourceStateMutex.unlock();
        g_resourceStateLocked = false;
	}

	void ResourceStateTracker::AddGlobalResourceState(ID3D12Resource* resource, D3D12_RESOURCE_STATES state) {
        if(resource != nullptr) {
            std::lock_guard<std::mutex> lock(g_resourceStateMutex);
            g_resourceState[resource].SetSubresourceState(D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, state);
        }
    }

	void ResourceStateTracker::RemoveGlobalResourceState(ID3D12Resource* resource) {
        if (resource != nullptr) {
            std::lock_guard<std::mutex> lock(g_resourceStateMutex);
            g_resourceState.erase(resource);
        }
	}

	ResourceStateTracker::ResourceStateTracker() {}

	ResourceStateTracker::~ResourceStateTracker() {}

	void ResourceStateTracker::ResourceBarrier(const D3D12_RESOURCE_BARRIER& barrier) {
        if (barrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION) {
            const D3D12_RESOURCE_TRANSITION_BARRIER& transitionBarrier = barrier.Transition;

            const auto iter = m_finalResourceState.find(transitionBarrier.pResource);
            if (iter != m_finalResourceState.end()) {
                auto& resourceState = iter->second;
                
                if (transitionBarrier.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES &&
                    !resourceState.SubresourceState.empty()) {
                    for (auto subresourceState : resourceState.SubresourceState) {
                        if (transitionBarrier.StateAfter != subresourceState.second) {
                            D3D12_RESOURCE_BARRIER newBarrier = barrier;
                            newBarrier.Transition.Subresource = subresourceState.first;
                            newBarrier.Transition.StateBefore = subresourceState.second;
                            m_resourceBarriers.push_back(newBarrier);
                        }
                    }
                } else  {
                    auto finalState = resourceState.GetSubresourceState(transitionBarrier.Subresource);
                    if (transitionBarrier.StateAfter != finalState) {
                        D3D12_RESOURCE_BARRIER newBarrier = barrier;
                        newBarrier.Transition.StateBefore = finalState;
                        m_resourceBarriers.push_back(newBarrier);
                    }
                }
            } else {
                m_pendingResourceBarriers.push_back(barrier);
            }

            m_finalResourceState[transitionBarrier.pResource].SetSubresourceState(transitionBarrier.Subresource, transitionBarrier.StateAfter);
        } else {
            m_resourceBarriers.push_back(barrier);
        }
	}

	void ResourceStateTracker::TransitionResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource) {
		if (resource) {
			ResourceBarrier(CD3DX12_RESOURCE_BARRIER::Transition(resource, D3D12_RESOURCE_STATE_COMMON, stateAfter, subResource));
		}
	}

	void ResourceStateTracker::TransitionResource(const Resource& resource, D3D12_RESOURCE_STATES stateAfter, UINT subResource) {
		TransitionResource(resource.Get().Get(), stateAfter, subResource);
	}

	void ResourceStateTracker::UAVBarrier(const Resource* resource) {
		ID3D12Resource* pResource = resource != nullptr ? resource->Get().Get() : nullptr;
		ResourceBarrier(CD3DX12_RESOURCE_BARRIER::UAV(pResource));
	}

	void ResourceStateTracker::AliasBarrier(const Resource* resourceBefore, const Resource* resourceAfter) {
		ID3D12Resource* pResourceBefore = resourceBefore != nullptr ? resourceBefore->Get().Get() : nullptr;
		ID3D12Resource* pResourceAfter = resourceAfter != nullptr ? resourceAfter->Get().Get() : nullptr;

		ResourceBarrier(CD3DX12_RESOURCE_BARRIER::Aliasing(pResourceBefore, pResourceAfter));
	}

	uint32_t ResourceStateTracker::FlushPendingResourceBarriers(CommandList& commandList) {
        assert(g_resourceStateLocked);

        std::vector<D3D12_RESOURCE_BARRIER> resourceBarriers;
        resourceBarriers.reserve(m_pendingResourceBarriers.size());
        for (auto pendingBarrier : m_pendingResourceBarriers) {
            if (pendingBarrier.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION) {
                auto pendingTransition = pendingBarrier.Transition;

                const auto& iter = g_resourceState.find(pendingTransition.pResource);
                if (iter != g_resourceState.end()) {
                    auto& resourceState = iter->second;
                    if (pendingTransition.Subresource == D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES &&
                        !resourceState.SubresourceState.empty())  {
                        
                        for (auto subresourceState : resourceState.SubresourceState) {
                            if (pendingTransition.StateAfter != subresourceState.second) {
                                D3D12_RESOURCE_BARRIER newBarrier = pendingBarrier;
                                newBarrier.Transition.Subresource = subresourceState.first;
                                newBarrier.Transition.StateBefore = subresourceState.second;
                                resourceBarriers.push_back(newBarrier);
                            }
                        }
                    } else {
                        auto globalState = (iter->second).GetSubresourceState(pendingTransition.Subresource);
                        if (pendingTransition.StateAfter != globalState) {
                            pendingBarrier.Transition.StateBefore = globalState;
                            resourceBarriers.push_back(pendingBarrier);
                        }
                    }
                }
            }
        }

        UINT numBarriers = static_cast<UINT>(resourceBarriers.size());
        if (numBarriers > 0) {
            auto d3d12CommandList = commandList.GraphicsCommandList();
            d3d12CommandList->ResourceBarrier(numBarriers, resourceBarriers.data());
        }

        m_pendingResourceBarriers.clear();
        return numBarriers;
	}

	void ResourceStateTracker::FlushResourceBarriers(CommandList& commandList) {
        UINT numBarriers = static_cast<UINT>(m_resourceBarriers.size());
        if (numBarriers > 0) {
            auto d3d12CommandList = commandList.GraphicsCommandList();
            d3d12CommandList->ResourceBarrier(numBarriers, m_resourceBarriers.data());
            m_resourceBarriers.clear();
        }
	}

	void ResourceStateTracker::CommitFinalResourceStates() {
        assert(g_resourceStateLocked);

        for (const auto& resourceState : m_finalResourceState) {
            g_resourceState[resourceState.first] = resourceState.second;
        }
        m_finalResourceState.clear();
	}

	void ResourceStateTracker::Reset() {
        m_pendingResourceBarriers.clear();
        m_resourceBarriers.clear();
        m_finalResourceState.clear();
	}

}