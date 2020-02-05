#pragma once
#include <mutex>
#include <memory>
#include <exception>
#include <stack>


class empty_stack : public std::exception
{
public:
	const char *what() const noexcept {
		return "stack is empty";
	}
};

template<typename T>
class thread_stack
{
private:
    cx_stack<T> stack;
	mutable std::recursive_mutex stack_mutex;
	

public:
	thread_stack() = default;

	thread_stack(const thread_stack& other) {
		std::lock_guard<std::recursive_mutex> lock(other.stack_mutex);
		stack = other.stack;
	}

	thread_stack& operator=(const thread_stack& other) = delete;


	void push(const T& val) {
		std::lock_guard<std::recursive_mutex> lock(stack_mutex);
		stack.push(val);
	}

	void push(T&& val) {
		std::lock_guard<std::recursive_mutex> lock(stack_mutex);
		stack.push(std::forward<T>(val));

	}

	bool empty() const {
		std::lock_guard<std::recursive_mutex> lock(stack_mutex);
		return stack.empty();
	}

	std::shared_ptr<T> pop() {
		std::lock_guard<std::recursive_mutex> lock(stack_mutex);
		if (empty()) {
			return std::shared_ptr<T>();
		}

		std::shared_ptr<T> ptr = 
			std::make_shared<T>(std::move(stack.top()));
		
		stack.pop();
		return ptr;
	}

	void pop(T& val) {
		std::lock_guard<std::recursive_mutex> lock(stack_mutex);
		if (empty()) {
			throw empty_stack();
		}

		val = std::move(stack.top());
		stack.pop();
	}
};















