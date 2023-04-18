#pragma once

#include <queue>
#include <mutex>

namespace collection {

	template<typename T>
	class ThreadSafeQueue {
		public:
			ThreadSafeQueue();
			ThreadSafeQueue(const ThreadSafeQueue& copy);

			void Push(T value);
			bool Pop(T& value);
			bool IsEmpty() const;
			size_t Size() const;

		private:
			std::queue<T>		m_queue;
			mutable std::mutex	m_mutex;
	};
}

template<typename T>
collection::ThreadSafeQueue<T>::ThreadSafeQueue() {}

template<typename T>
collection::ThreadSafeQueue<T>::ThreadSafeQueue(const collection::ThreadSafeQueue<T>& copy) {
    std::lock_guard<std::mutex> lock(copy.m_mutex);
    m_queue = copy.m_queue;
}

template<typename T>
void collection::ThreadSafeQueue<T>::Push(T value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_queue.push(std::move(value));
}

template<typename T>
bool collection::ThreadSafeQueue<T>::Pop(T& value) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_queue.empty()) {
        return false;
    }
        
    value = m_queue.front();
    m_queue.pop();

    return true;
}

template<typename T>
bool collection::ThreadSafeQueue<T>::IsEmpty() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.empty();
}

template<typename T>
size_t collection::ThreadSafeQueue<T>::Size() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_queue.size();
}