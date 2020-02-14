#pragma once
#include <mutex>


template<typename T>
class thread_queue
{
private:
	struct node
	{
		T data;
		node *next;
	};

	node *head;
	node *tail;			//指向空白节点
	std::mutex head_mutex;
	std::mutex tail_mutex;

	std::condition_variable cond;

	node *get_tail();
	node *pop_head();	//并未上锁
	std::unique_lock<std::mutex> wait_for_data();

public:
	thread_queue(): head(new node), tail(head) {
		head->next = nullptr;
	}
	thread_queue(const thread_queue& other) = delete;
	thread_queue& operator=(const thread_queue& other) = delete;
	~thread_queue();

	std::shared_ptr<T> try_pop();
	bool try_pop(T& val);
	std::shared_ptr<T> wait_pop();
	void wait_pop(T& val);
	void push(const T& val);
	void push(T&& val);
	bool empty();

};


template<typename T>
void thread_queue<T>::push(const T& val)
{
	node *new_tail = new node;
	new_tail->next = nullptr;

	{
		std::lock_guard<std::mutex> lock(tail_mutex);
		tail->data = val;
		tail->next = new_tail;
		tail = new_tail;
	}

	cond.notify_one();
}


template<typename T>
void thread_queue<T>::push(T&& val)
{
	node *new_tail = new node;
	new_tail->next = nullptr;

	{
		std::lock_guard<std::mutex> lock(tail_mutex);
		tail->data = std::move(val);
		tail->next = new_tail;
		tail = new_tail;
	}

	cond.notify_one();
}

template<typename T>
typename thread_queue<T>::node *
thread_queue<T>::get_tail()
{
	std::lock_guard<std::mutex> lock(tail_mutex);
	return tail;
}


template<typename T>
typename thread_queue<T>::node *
thread_queue<T>::pop_head()
{
	node *old_head = head;
	head = old_head->next;
	return old_head;
}


template<typename T>
std::unique_lock<std::mutex>
thread_queue<T>::wait_for_data()
{
	std::unique_lock<std::mutex> lock(head_mutex);
	cond.wait(lock, [&]() {
		return head != get_tail();
	});

	return std::move(lock);
}


template<typename T>
std::shared_ptr<T>
thread_queue<T>::wait_pop()
{
	std::unique_lock<std::mutex> lock(wait_for_data());
	return std::make_shared<T>(
		std::move(pop_head()->data));
}

template<typename T>
void thread_queue<T>::wait_pop(T& val)
{
	std::unique_lock<std::mutex> lock(wait_for_data());
	val = std::move(pop_head()->data);
}


template<typename T>
std::shared_ptr<T>
thread_queue<T>::try_pop()
{
	std::lock_guard<std::mutex> lock(head_mutex);
	if (head == get_tail()) {
		return std::shared_ptr<T>();
	}

	return std::make_shared<T>(
		std::move(pop_head()->data));
}


template<typename T>
bool thread_queue<T>::try_pop(T& val)
{
	std::lock_guard<std::mutex> lock(head_mutex);
	if (head == get_tail()) {
		return false;
	}

	val = std::move(pop_head()->data);
	return true;
}


template<typename T>
bool thread_queue<T>::empty()
{
	std::lock_guard<std::mutex> lock(head_mutex);
	return head == get_tail();
}

template<typename T>
thread_queue<T>::~thread_queue()
{
	for (node *cur = head; cur != nullptr;)
	{
		node *prev = cur;
		cur = cur->next;
		delete prev;
	}
}

