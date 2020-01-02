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
	cx_stack() { container = cx_deque<T>(); }
	explicit cx_stack(const container_type& con) { container = con; }
	explicit cx_stack(container_type&& con) { 
		container = std::move(con); 
	}

	bool empty() const { return container.empty(); }
	size_type size() const { return container.size(); }
	reference top() { return container.back(); }
	void push(const value_type& val) {  container.push_back(val); }
	void pop() { container.pop_back(); }

	friend bool operator==<>(const cx_stack& lhs, const cx_stack& rhs);
	friend bool operator!=<>(const cx_stack& lhs, const cx_stack& rhs);

protected:
	Container container;
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











