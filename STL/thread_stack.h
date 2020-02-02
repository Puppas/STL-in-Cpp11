#pragma once
#include "free_list_allocator.h"
#include "cx_stack.h"
#include <mutex>
#include <memory>
#include <exception>


class empty_stack_error : public std::exception
{
	const char* what() const noexcept {
		return "stack is empty";
	}
};


template<typename T, typename Alloc = free_list_allocator<T>>
class thread_stack
{
private:
	cx_stack<T, Alloc> stack;
	mutable std::mutex stack_mutex;

public:
	thread_stack() = default;

	thread_stack(const thread_stack& other) {
		std::lock_guard<std::mutex> lock(other.stack_mutex);
		stack = other.stack;
	}

	thread_stack& operator=(const thread_stack& other) = delete;


	void push(const T& val) {
		std::lock_guard<std::mutex> lock(stack_mutex);
		stack.push(val);
	}

	bool empty() const {
		std::lock_guard<std::mutex> lock(stack_mutex);
		return stack.empty();
	}

	std::shared_ptr<T> pop() {
		std::lock_guard<std::mutex> lock(stack_mutex);
		if (empty()) {
			throw empty_stack_error();
		}

		std::shared_ptr<T> ptr = 
			std::make_shared<T>(std::move(stack.top()));
		
		stack.pop();
		return ptr;
	}

	void pop(T& val) {
		std::lock_guard<std::mutex> lock(stack_mutex);
		if (empty()) {
			throw empty_stack_error();
		}

		val = std::move(stack.top());
		stack.pop();
	}
};















