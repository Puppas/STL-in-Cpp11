#pragma once

#include <utility>
#include <memory>
#include "thread_queue.h"
#include <atomic>
#include "jthread.h"
#include <future>
#include <queue>
#include <deque>


class function_wrapper
{
private:
	class impl_base
	{
	public:
		virtual void call() = 0;
		virtual ~impl_base() {}
	};

	template<typename Function, typename... Args>
	class wrapper_impl: public impl_base
	{
	public:
		using bind_type = decltype(std::bind(std::declval<Function>(),
			std::declval<Args>()...));
	private:
		bind_type bind_function;
	public:
		wrapper_impl(Function&& f, Args&&... args): 
			bind_function(std::forward<Function>(f), std::forward<Args>(args)...) 
		{}

		void call() {
			bind_function();
		}
	};

	std::unique_ptr<impl_base> pimpl;

public:
	function_wrapper() = default;

	template<typename Function, typename... Args>
	function_wrapper(Function&& f, Args&&... args):
		pimpl(new wrapper_impl<Function, Args...>(
			std::forward<Function>(f), std::forward<Args>(args)...))
	{}

	function_wrapper(const function_wrapper&) = delete;
	function_wrapper& operator=(const function_wrapper&) = delete;

	function_wrapper(function_wrapper&& other) noexcept:
		pimpl(std::move(other.pimpl)) {}
	function_wrapper& operator=(function_wrapper&& other) noexcept {
		pimpl = std::move(other.pimpl);
		return *this;
	}

	void operator()() {
		pimpl->call();
	}
};


class task_steal_queue
{
private:
	std::deque<function_wrapper> queue;
	mutable std::mutex queue_mutex;

public:
	task_steal_queue() = default;
	task_steal_queue(const task_steal_queue&) = delete;
	task_steal_queue& operator=(const task_steal_queue&) = delete;
	
	void push(function_wrapper f) {
		std::lock_guard<std::mutex> lock(queue_mutex);
		queue.push_front(std::move(f));
	}

	bool empty() const {
		std::lock_guard<std::mutex> lock(queue_mutex);
		return queue.empty();
	}

	std::size_t size() const {
		std::lock_guard<std::mutex> lock(queue_mutex);
		return queue.size();
	}

	bool try_pop(function_wrapper& res) {
		std::lock_guard<std::mutex> lock(queue_mutex);
		if (queue.empty())
			return false;

		res = std::move(queue.front());
		queue.pop_front();
		return true;
	}

	bool try_steal(function_wrapper& res) {
		std::lock_guard<std::mutex> lock(queue_mutex);
		if (queue.empty())
			return false;

		res = std::move(queue.back());
		queue.pop_back();
		return true;
	}
};


class thread_pool
{
private:
	std::atomic_bool done;
	thread_queue<function_wrapper> global_task_queue;

	std::vector<std::unique_ptr<task_steal_queue>>
		local_task_queue_vec;
	std::vector<jthread> threads;

	static thread_local task_steal_queue*
		local_task_queue_ptr;
	static thread_local std::size_t queue_index;


	void thread_work(std::size_t index);

	bool pop_task_from_local_queue(function_wrapper& task) {
		return local_task_queue_ptr &&
			local_task_queue_ptr->try_pop(task);
	}

	bool pop_task_from_global_queue(function_wrapper& task) {
		return global_task_queue.try_pop(task);
	}

	bool pop_task_from_other_queue(function_wrapper& task);

public:
	thread_pool();

	~thread_pool() {
		done = true;
	}

	template<typename Function, typename... Args>
	std::future<std::invoke_result_t<Function, Args...>>
	submit(Function&& f, Args&&... args);

	void run_task();
};



bool thread_pool::pop_task_from_other_queue(
	function_wrapper& task)
{
	std::size_t vec_size = local_task_queue_vec.size();
	for (std::size_t i = 0; i < vec_size; ++i)
	{
		std::size_t index = (queue_index + i + 1) % vec_size;
		if (local_task_queue_vec[index]->try_steal(task))
			return true;
	}

	return false;
}


void thread_pool::thread_work(std::size_t index)
{
	queue_index = index;
	local_task_queue_ptr = local_task_queue_vec[queue_index].get();

	while (!done)
	{
		run_task();
	}
}

thread_pool::thread_pool() : done(false)
{
	const std::size_t thread_count = std::thread::hardware_concurrency();

	try {
		for (std::size_t i = 0; i < thread_count; ++i)
		{
			local_task_queue_vec.push_back(
				std::unique_ptr<task_steal_queue>(
					new task_steal_queue));
		
			threads.push_back(jthread(
				&thread_pool::thread_work, this, i));
		}
	}
	catch (...) {
		done = true;
		throw;
	}
}

template<typename Function, typename... Args>
std::future<std::invoke_result_t<Function, Args...>>
thread_pool::submit(Function&& f, Args&&... args)
{
	using result_type = std::invoke_result_t<Function, Args...>;

	std::packaged_task<result_type(Args...)> task(
		std::forward<Function>(f), std::forward<Args>(args)...);
	std::future<result_type> result = task.get_future();

	if (local_task_queue_ptr) {
		local_task_queue_ptr->push(std::move(task));
	}
	else {
		global_task_queue.push(std::move(task));
	}

	return result;
}


void thread_pool::run_task()
{
	function_wrapper task;
	if (pop_task_from_local_queue(task) ||
		pop_task_from_global_queue(task) ||
		pop_task_from_other_queue(task)) {
		
		task();
	}
	else {
		std::this_thread::yield();
	}
}


thread_local task_steal_queue*
thread_pool::local_task_queue_ptr = nullptr;

thread_local std::size_t thread_pool::queue_index = -1;