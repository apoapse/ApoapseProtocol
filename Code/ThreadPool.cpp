#include "stdafx.h"
#include "ThreadPool.h"
#include "Common.h"

ThreadPool::ThreadPool(const std::string& threadPoolName, UInt32 nbThreads)
	: m_threadPoolName(threadPoolName)
	, m_queueCapacity(INITIAL_QUEUE_CAPACITY)
	, m_tasksInQueueCounter(0)
{
	for (UInt32 i = 0; i < nbThreads; i++)
	{
		std::thread thread([this] { this->Consume(); });
		thread.detach();

		m_workerThreads.push_back(std::move(thread));
	}

	LOG << "Created the thread pool " << threadPoolName << " with " << nbThreads << " worker threads";
}

ThreadPool::~ThreadPool()
{
	ITask* task;

	while (m_workQueue.pop(task))
		delete task;
}

void ThreadPool::OnAddedTask()
{
	++m_tasksInQueueCounter;

	ASSERT(m_tasksInQueueCounter > 0);

	m_conditionVariable.notify_one();

	if (m_tasksInQueueCounter >= (Int64)(m_queueCapacity - 2))	// Margin of 2 items
	{
		Int64 newSize = m_queueCapacity * 2;

		LOG << "Thread pool " << m_threadPoolName << ": queue close to be full, preventively resizing to " << newSize;
		ResizeQueue(newSize);
	}
}

void ThreadPool::Consume()
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		if (m_tasksInQueueCounter > 0)
		{
			ITask* task;
			if (m_workQueue.pop(task))
			{
				ASSERT(m_tasksInQueueCounter > 0);
				--m_tasksInQueueCounter;

				task->Execute();
				delete task;
			}
		}

		if (m_tasksInQueueCounter == 0)
			m_conditionVariable.wait(lock);
	}
}

void ThreadPool::ResizeQueue(size_t requestedQueueCapacity)
{
	ASSERT_MSG(requestedQueueCapacity >= INITIAL_QUEUE_CAPACITY, "JobManager: Requested queue capacity should not be lower than the default value of DEFAULT_QUEUE_CAPACITY");

	m_workQueue.reserve(requestedQueueCapacity);

	m_queueCapacity = m_queueCapacity + (Int64)requestedQueueCapacity;
}

Int64 ThreadPool::GetTasksInQueueCount() const
{
	return m_tasksInQueueCounter;
}

std::string ThreadPool::GetThreadPoolName() const
{
	return m_threadPoolName;
}
