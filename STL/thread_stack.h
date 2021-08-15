#pragma once
#include <mutex>
#include <memory>
#include <exception>
#include <stack>


template<typename T, typename Container = cx_deque<T>>
class thread_stack
{
public:
	using value_type = T;
	using container_type = Container;
	using reference = T&;
	using const_reference = const reference;
	using size_type = std::size_t;

private:
    cx_stack<T, Container> stack;
	mutable std::recursive_mutex stack_mutex;
	

public:
	thread_stack() = default;

	thread_stack(const thread_stack& other) {
		std::lock_guard<std::recursive_mutex> lock(other.stack_mutex);
		stack = other.stack;
	}

	thread_stack& operator=(const thread_stack& other) = delete;


	void push(T val) {
		std::lock_guard<std::recursive_mutex> lock(stack_mutex);
		stack.push(std::move(val));
	}

	bool empty() const{
		std::lock_guard<std::recursive_mutex> lock(stack_mutex);
		return stack.empty();
	}

	size_type size() const{
		std::lock_guard<std::recursive_mutex> lock(stack_mutex);
		return stack.size();
	}

	reference top() {
		std::lock_guard<std::recursive_mutex> lock(stack_mutex);
		return stack.top();
	}

	const_reference top() const {
		std::lock_guard<std::recursive_mutex> lock(stack_mutex);
		return stack.top();
	}

	std::shared_ptr<T> pop() {
		std::lock_guard<std::recursive_mutex> lock(stack_mutex);
		if (empty()) {
			throw std::exception("pop when stack is empty");
		}

		std::shared_ptr<T> ptr = 
			std::make_shared<T>(std::move(stack.top()));
		stack.pop();
		return ptr;
	}

	void pop(T& val) {
		std::lock_guard<std::recursive_mutex> lock(stack_mutex);
		if (empty()) {
			throw std::exception("pop when stack is empty");
		}

		val = std::move(stack.top());
		stack.pop();
	}
};















