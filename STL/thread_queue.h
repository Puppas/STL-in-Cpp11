#pragma once
#include <mutex>


template<typename T>
class thread_queue
{
public:
	using value_type = T;
	using reference = T&;
	using const_reference = const reference;
	using size_type = std::size_t;

private:
	struct node
	{
		std::shared_ptr<T> data;
		std::unique_ptr<node> next;
	};

	std::unique_ptr<node> head;
	node* tail;			//指向空白节点
	size_type queue_size;

	mutable std::mutex head_mutex;
	mutable std::mutex tail_mutex;
	mutable std::mutex size_mutex;
	std::condition_variable data_cond;

	std::unique_ptr<node> pop_head() noexcept;	
	std::unique_lock<std::mutex> wait_for_data();
	void inc_size();
	void dec_size();

public:
	thread_queue(): head(new node), tail(head.get()), queue_size(0)
	{}
	thread_queue(const thread_queue& other) = delete;
	thread_queue& operator=(const thread_queue& other) = delete;

	std::shared_ptr<T> try_pop();
	bool try_pop(T& val);
	std::shared_ptr<T> wait_pop();
	void wait_pop(T& val);

 	void push(T val);
	bool empty() const;
	size_type size() const;
};


template<typename T>
void thread_queue<T>::inc_size()
{
	std::lock_guard<std::mutex> lock(size_mutex);
	++queue_size;
}

template<typename T>
void thread_queue<T>::dec_size()
{
	std::lock_guard<std::mutex> lock(size_mutex);
	--queue_size;
}


template<typename T>
void thread_queue<T>::push(T val)
{
	std::shared_ptr<T> data = std::make_shared<T>(std::move(val));
	std::unique_ptr<node> new_tail(new node);
	{
		std::lock_guard<std::mutex> lock(tail_mutex);
		tail->data = data;
		tail->next = std::move(new_tail);
		tail = tail->next.get();
	}

	inc_size();
	data_cond.notify_one();
}


template<typename T>
std::unique_ptr<typename thread_queue<T>::node>
thread_queue<T>::pop_head() noexcept
{
	std::unique_ptr<node> old_head = std::move(head);
	head = std::move(old_head->next);
	return old_head;
}


template<typename T>
std::unique_lock<std::mutex>
thread_queue<T>::wait_for_data()
{
	std::unique_lock<std::mutex> lock(head_mutex);
	data_cond.wait(lock, [&]() {
		std::unique_lock<std::mutex> lock(size_mutex);
		return queue_size > 0;
	});

	return std::move(lock);
}


template<typename T>
std::shared_ptr<T>
thread_queue<T>::wait_pop()
{
	std::unique_lock<std::mutex> lock(wait_for_data());
	std::shared_ptr<T> ptr = pop_head()->data;
	lock.unlock();
	dec_size();
	return ptr;
}


template<typename T>
void thread_queue<T>::wait_pop(T& val)
{
	std::unique_lock<std::mutex> lock(wait_for_data());
	val = std::move(*(pop_head()->data));
	lock.unlock();
	dec_size();
}


template<typename T>
std::shared_ptr<T>
thread_queue<T>::try_pop()
{
	std::unique_lock<std::mutex> lock(head_mutex);
	{
		std::lock_guard<std::mutex> lock(size_mutex);
		if (queue_size == 0) {
			return std::shared_ptr<T>();
		}
	}

	std::shared_ptr<T> ptr = pop_head()->data;
	lock.unlock();
	dec_size();
	return ptr;
}


template<typename T>
bool thread_queue<T>::try_pop(T& val)
{
	std::unique_lock<std::mutex> lock(head_mutex);
	{
		std::lock_guard<std::mutex> lock(size_mutex);
		if (queue_size <= 0) {
			return false;
		}
	}
	val = std::move(*pop_head()->data);
	lock.unlock();
	dec_size();
	return true;
}


template<typename T>
bool thread_queue<T>::empty() const
{
	std::lock_guard<std::mutex> lock(size_mutex);
	return queue_size == 0;
}

template<typename T>
typename thread_queue<T>::size_type 
thread_queue<T>::size() const
{
	std::lock_guard<std::mutex> lock(size_mutex);
	return queue_size;
}
