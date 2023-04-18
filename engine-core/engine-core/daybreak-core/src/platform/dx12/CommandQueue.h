#pragma once

#include "common/ThreadSafeQueue.h"

namespace dx12 {

	class CommandList;

	class DAYBREAK_API CommandQueue {

		public:
			CommandQueue(D3D12_COMMAND_LIST_TYPE type);
			virtual ~CommandQueue();

			void Initialize(ComPtr<ID3D12Device2> device);

			uint64_t ExecuteCommandList(std::shared_ptr<CommandList> commandList);
			uint64_t ExecuteCommandLists(const std::vector<std::shared_ptr<CommandList>>& commandLists);

			uint64_t Signal();
			bool IsFenceComplete(uint64_t fenceValue);
			void WaitForFenceValue(uint64_t fenceValue);
			void Flush();

			void Wait(const CommandQueue& other);

			ComPtr<ID3D12CommandQueue> D3D12CommandQueue() const;
			std::shared_ptr<CommandList> CommandList();

	private:
		void ProccessInFlightCommandLists();

		using CommandListEntry = std::tuple<uint64_t, std::shared_ptr<dx12::CommandList>>;

		D3D12_COMMAND_LIST_TYPE							m_type;
		ComPtr<ID3D12CommandQueue>						m_queue;
		ComPtr<ID3D12Fence>								m_fence;
		uint64_t										m_fenceValue;

		collection::ThreadSafeQueue<CommandListEntry>					m_inFlightCommandLists;
		collection::ThreadSafeQueue<std::shared_ptr<dx12::CommandList>>	m_availableCommandLists;

		std::thread				m_processInFlightCommandListsThread;
		std::atomic_bool		m_processInFlightCommandLists;
		std::mutex				m_processInFlightCommandListsThreadMutex;
		std::condition_variable	m_processInFlightCommandListsThreadCV;
	};
}