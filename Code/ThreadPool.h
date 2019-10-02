#pragma once
#include <atomic>
#include <thread>
#include <condition_variable>
#include <future>
#include <boost/lockfree/queue.hpp>
#include "TypeDefs.hpp"

#define INITIAL_QUEUE_CAPACITY 512

class ThreadPool
{
	struct ITask
	{
		virtual ~ITask() = default;
		virtual void Execute() = 0;
	};

	template <typename T>
	struct Task final : public ITask
	{
		std::packaged_task<T()> task;

		Task(std::function<T()>&& func) : task(func)
		{
		}

		void Execute() override
		{
			task();
		}
	};

	struct SimpleTask : public ITask
	{
		const std::function<void()> task;

		SimpleTask(const std::function<void()>& func) : task(func)
		{}
		
		void Execute() override
		{
			task();
		}
	};

	const std::string m_threadPoolName;
	std::vector<std::thread> m_workerThreads;
	std::atomic<Int64> m_tasksInQueueCounter;
	std::atomic<UInt64> m_queueCapacity;
	boost::lockfree::queue<ITask*> m_workQueue{ INITIAL_QUEUE_CAPACITY };	// We are using raw pointer here because the Boost lockfree queue require a trivial assignment operator
	std::condition_variable m_conditionVariable;
	std::mutex m_mutex;

public:
	//************************************
	// Method:    ThreadPool::ThreadPool - Task based thread pool with a producer-consumer queue model
	// Parameter: const string & threadPoolName - Name of the object, useful for debugging
	// Parameter: UInt32 nbThreads - Number of worker threads
	//************************************
	ThreadPool(const std::string& threadPoolName, UInt32 nbThreads, bool autoRun);
	virtual ~ThreadPool();

	//************************************
	// Method:    ThreadPool::PushTask - Push a task to the queue, generaly a lambda
	// Parameter: const T func - callable object like std::function
	// WARNING:   For the return class, std::promise need a default constructor and an copy assignement operator
	//************************************
	template<typename T>
	std::future<typename std::result_of<T()>::type> PushTaskFuture(T func)
	{
		using TypeResult = typename std::result_of<T()>::type;
		static_assert((!std::is_object<TypeResult>::value || std::is_default_constructible<TypeResult>::value/* && std::is_trivially_assignable<TypeResult, TypeResult>::value*/), "The type provided need to have a default constructor and trivial assignment operator");

		auto* task = new Task<TypeResult>(std::move(func));
		std::future<TypeResult> response = task->task.get_future();

		while (!m_workQueue.push(std::move(task)));
		OnAddedTask();

		return response;
	}

	void PushTask(const std::function<void()>& func)
	{
		auto* task = new SimpleTask(func);
		while (!m_workQueue.push(std::move(task)));
		OnAddedTask();
	}
	
	Int64 GetTasksInQueueCount() const;
	std::string GetThreadPoolName() const;

	void Run();

private:
	void OnAddedTask();
	void ResizeQueue(size_t);
};