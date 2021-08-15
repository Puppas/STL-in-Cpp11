#pragma once
#include "cx_deque.h"

template<typename T, typename Container = cx_deque<T>>
class cx_stack
{
public:
	using value_type = T;
	using container_type = Container;
	using reference = T & ;
	using const_reference = const reference;
	using size_type = std::size_t;

public:
	cx_stack() { container = Container(); }
	cx_stack(const container_type& con) { container = con; }
	cx_stack(container_type&& con) { 
		container = std::move(con); 
	}
	cx_stack(cx_stack&& stack) {
		container = std::move(stack.container);
	}

	cx_stack& operator=(cx_stack&& stack) {
		container = std::move(stack.container);
		return *this;
	}


	bool empty() const noexcept { return container.empty(); }
	size_type size() const noexcept { return container.size(); }
	reference top(){ return container.back(); }
	const_reference top() const { return container.back(); }
	void push(const value_type& val) {  container.push_back(val); }
	void pop() { container.pop_back(); }

	void swap(cx_stack& stack) noexcept{
		this->container.swap(stack.container);
	}

	template<typename U, typename ContainerType>
	friend void swap(cx_stack<U, ContainerType>& lhs,
					 cx_stack<U, ContainerType>& rhs)
	{
		lhs.swap(rhs);
	}

	friend bool operator==<>(const cx_stack& lhs, const cx_stack& rhs);
	friend bool operator!=<>(const cx_stack& lhs, const cx_stack& rhs);

protected:
	container_type container;
};


template<typename T, typename Container>
bool operator==(const cx_stack<T, Container>& lhs,
				const cx_stack<T, Container>& rhs)
{
	return lhs.container == rhs.container;
}

template<typename T, typename Container>
bool operator!=(const cx_stack<T, Container>& lhs,
				const cx_stack<T, Container>& rhs)
{
	return !(lhs == rhs);
}











