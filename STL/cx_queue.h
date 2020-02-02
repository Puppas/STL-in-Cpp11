#pragma once
#include "cx_deque.h"

template<typename T, typename Container = cx_deque<T>>
class cx_queue
{
public:
	using value_type = T;
	using container_type = Container;
	using reference = T & ;
	using const_reference = const reference;
	using size_type = std::size_t;


public:
	cx_queue() {
		container = container_type();
	}

	explicit cx_queue(const container_type& con) {
		container = con;
	}

	explicit cx_queue(container_type&& con) {
		container = std::move(con);
	}
	
	explicit cx_queue(cx_queue&& queue) {
		container = std::move(queue.container);
	}

	cx_queue& operator=(cx_queue&& queue) {
		container = std::move(queue.container);
		return *this;
	}

	bool empty() const { return container.empty(); }
	size_type size() const { return container.size(); }
	
	reference front() { return container.front(); }
	const_reference front() const { return container.front(); }
	reference back() { return container.back(); }
	const_reference back() const { return container.back(); }

	void pop() { container.pop_front(); }
	void push(const value_type& val) { container.push_back(val); }

	void swap(cx_queue& queue) {
		this->container.swap(queue.container);
	}

	template<typename U, typename ContainerType>
	friend void swap(cx_queue<U, ContainerType>& lhs,
					 cx_queue<U, ContainerType>& rhs) {
		lhs.swap(rhs);
	}

	friend bool operator==<>(const cx_queue& lhs, const cx_queue& rhs);
	friend bool operator!=<>(const cx_queue& lhs, const cx_queue& rhs);

protected:
	container_type container;
};


template<typename T, typename Container>
bool operator==(const cx_queue<T, Container>& lhs,
				const cx_queue<T, Container>& rhs)
{
	return lhs.container == rhs.container;
}

template<typename T, typename Container>
bool operator!=(const cx_queue<T, Container>& lhs,
				const cx_queue<T, Container>& rhs)
{
	return !(lhs == rhs);
}











