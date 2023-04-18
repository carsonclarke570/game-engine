#include "daybreak.h"
#include "CommandQueue.h"

#include "CommandList.h"
#include "ResourceStateTracker.h"

namespace dx12 {
	CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE type) : 
		m_fenceValue(0), 
		m_type(type), 
		m_queue(nullptr), 
		m_fence(nullptr), 
		m_processInFlightCommandLists(true) {
	}

	CommandQueue::~CommandQueue() {
		m_processInFlightCommandLists = false;
		m_processInFlightCommandListsThread.join();
	}

	void CommandQueue::Initialize(ComPtr<ID3D12Device2> device) {
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = m_type;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		ThrowOnFailure(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_queue)));
		ThrowOnFailure(device->CreateFence(m_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

		switch (m_type) {
			case D3D12_COMMAND_LIST_TYPE_COPY:
				m_queue->SetName(L"Copy Command Queue");
				break;
			case D3D12_COMMAND_LIST_TYPE_COMPUTE:
				m_queue->SetName(L"Compute Command Queue");
				break;
			case D3D12_COMMAND_LIST_TYPE_DIRECT:
				m_queue->SetName(L"Direct Command Queue");
				break;
		}

		m_processInFlightCommandListsThread = std::thread(&CommandQueue::ProccessInFlightCommandLists, this);
	}

	uint64_t CommandQueue::ExecuteCommandList(std::shared_ptr<dx12::CommandList> commandList) {
		return ExecuteCommandLists(std::vector<std::shared_ptr<dx12::CommandList>>({ commandList }));
	}

	uint64_t CommandQueue::ExecuteCommandLists(const std::vector<std::shared_ptr<dx12::CommandList>>& commandLists) {
		ResourceStateTracker::Lock();

		// Command lists that need to put back on the command list queue.
		std::vector<std::shared_ptr<dx12::CommandList>> toBeQueued;
		toBeQueued.reserve(commandLists.size() * 2);

		// Generate mips command lists.
		std::vector<std::shared_ptr<dx12::CommandList>> generateMipsCommandLists;
		generateMipsCommandLists.reserve(commandLists.size());

		// Command lists that need to be executed.
		std::vector<ID3D12CommandList*> d3d12CommandLists;
		d3d12CommandLists.reserve(commandLists.size() * 2);

		for (auto commandList : commandLists) {
			auto pendingCommandList = CommandList();
			bool hasPendingBarriers = commandList->Close(*pendingCommandList);
			pendingCommandList->Close();

			// If there are no pending barriers on the pending command list, there is no reason to 
			// execute an empty command list on the command queue.
			if (hasPendingBarriers) {
				d3d12CommandLists.push_back(pendingCommandList->GraphicsCommandList().Get());
			}
			d3d12CommandLists.push_back(commandList->GraphicsCommandList().Get());

			toBeQueued.push_back(pendingCommandList);
			toBeQueued.push_back(commandList);

			/*auto generateMipsCommandList = commandList->GetGenerateMipsCommandList();
			if (generateMipsCommandList) {
				generateMipsCommandLists.push_back(generateMipsCommandList);
			}*/
		}

		UINT numCommandLists = static_cast<UINT>(d3d12CommandLists.size());
		m_queue->ExecuteCommandLists(numCommandLists, d3d12CommandLists.data());
		uint64_t fenceValue = Signal();

		ResourceStateTracker::Unlock();

		// Queue command lists for reuse.
		for (auto commandList : toBeQueued) {
			m_inFlightCommandLists.Push({ fenceValue, commandList });
		}

		// If there are any command lists that generate mips then execute those
		// after the initial resource command lists have finished.
		if (generateMipsCommandLists.size() > 0) {
			auto computeQueue = Application::Get()->CommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE);
			computeQueue->Wait(*this);
			computeQueue->ExecuteCommandLists(generateMipsCommandLists);
		}

		return fenceValue;
	}

	std::shared_ptr<CommandList> CommandQueue::CommandList() {
		std::shared_ptr<dx12::CommandList> commandList;

		if (!m_availableCommandLists.IsEmpty()) {
			m_availableCommandLists.Pop(commandList);
		} else {
			commandList = std::make_shared<dx12::CommandList>(m_type);
		}
		return commandList;
	}

	uint64_t CommandQueue::Signal() {
		uint64_t fenceValueForSignal = ++m_fenceValue;
		ThrowOnFailure(m_queue->Signal(m_fence.Get(), fenceValueForSignal));
		return fenceValueForSignal;
	}

	bool CommandQueue::IsFenceComplete(uint64_t fenceValue) {
		return m_fence->GetCompletedValue() >= fenceValue;
	}

	void CommandQueue::WaitForFenceValue(uint64_t fenceValue) {
		if (!IsFenceComplete(fenceValue)) {
			auto event = CreateEvent(NULL, FALSE, FALSE, NULL);
			assert(event && "Failed to create fence event handle.");

			m_fence->SetEventOnCompletion(fenceValue, event);
			WaitForSingleObject(event, DWORD_MAX);

			CloseHandle(event);
		}
	}

	void CommandQueue::Flush() {
		std::unique_lock<std::mutex> lock(m_processInFlightCommandListsThreadMutex);
		m_processInFlightCommandListsThreadCV.wait(lock, [this] { return m_inFlightCommandLists.IsEmpty(); });
		WaitForFenceValue(m_fenceValue);
	}

	void CommandQueue::Wait(const CommandQueue& other) {
		m_queue->Wait(other.m_fence.Get(), other.m_fenceValue);
	}

	ComPtr<ID3D12CommandQueue> CommandQueue::D3D12CommandQueue() const {
		return m_queue;
	}

	void CommandQueue::ProccessInFlightCommandLists() {
		std::unique_lock<std::mutex> lock(m_processInFlightCommandListsThreadMutex, std::defer_lock);

		while (m_processInFlightCommandLists) {
			CommandListEntry commandListEntry;

			lock.lock();
			while (m_inFlightCommandLists.Pop(commandListEntry))
			{
				auto fenceValue = std::get<0>(commandListEntry);
				auto commandList = std::get<1>(commandListEntry);

				WaitForFenceValue(fenceValue);

				commandList->Reset();

				m_availableCommandLists.Push(commandList);
			}
			lock.unlock();
			m_processInFlightCommandListsThreadCV.notify_one();

			std::this_thread::yield();
		}
	}
}