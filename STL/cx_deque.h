#pragma once
#include "free_list_allocator.h"
#include "alloc_destroy.h"
#include <iterator>
#include <cstdlib>
#include <initializer_list>

template<typename T, typename Alloc = free_list_allocator<T>>
class cx_deque
{
public:
	template<typename U, typename Ref = U & , typename Ptr = U * >
	struct deque_iterator
	{
		using iterator = deque_iterator<U, Ref, Ptr>;
		using iterator_category = std::random_access_iterator_tag;
		using value_type = U;
		using pointer = Ptr;
		using reference = Ref;
		using difference_type = std::ptrdiff_t;
		using map_pointer = pointer * ;
		using size_type = std::size_t;
	
		U *cur;
		U *first;          //当前缓冲区第一个元素
		U *last;		//当前缓冲区最后元素的下一位置
		map_pointer node;
		size_type buf_size = sizeof(U) >= 128 ? 1 : 128 / sizeof(U);

		deque_iterator() noexcept: cur(nullptr), first(nullptr),
			last(nullptr), node(nullptr) {}
		deque_iterator(pointer cur, pointer first, pointer last,
			map_pointer node) : cur(cur), first(first),
			last(last), node(node) {}


		bool operator==(const iterator& iter) const noexcept
		{
			return cur == iter.cur;
		}
		bool operator!=(const iterator& iter) const noexcept
		{
			return cur != iter.cur;
		}
		bool operator<(const iterator& iter) const noexcept
		{
			return node == iter.node ? cur < iter.cur : node < iter.node;
		}
		bool operator>(const iterator& iter) const noexcept
		{
			return node == iter.node ? cur > iter.cur : node > iter.node;
		}

		reference operator*() const noexcept { return *cur; }
		pointer operator->() const noexcept { return this->cur; }


		template<typename I>
		friend deque_iterator<I> 
		operator+(deque_iterator<I> iter, size_type n)
		{
			if (n < iter.last - iter.cur) {
				iter.cur += n;
			}
			else {
				size_type jump = 1 + (n - (iter.last - iter.cur)) / iter.buf_size;
				iter.node += jump;
				iter.cur = *iter.node + (n - (iter.last - iter.cur)) % iter.buf_size;
				iter.first = *iter.node;
				iter.last = *iter.node + iter.buf_size;
			}

			return iter;
		}

		iterator operator-(size_type n) const
		{
			iterator tmp = *this;

			if (n <= tmp.cur - tmp.first) {
				tmp.cur -= n;
			}
			else {
				size_type jump = 1 + (n - (tmp.cur - tmp.first + 1)) / buf_size;
				tmp.node -= jump;
				tmp.last = *tmp.node + tmp.buf_size;
				tmp.cur = tmp.last - 1 - (n - (tmp.cur - tmp.first + 1)) % buf_size;
				tmp.first = *tmp.node;
			}

			return tmp;
		}
		difference_type operator-(const iterator& iter) const
		{
			difference_type count = 0;
			if (node == iter.node)
			{
				count += cur - iter.cur;
				return count;
			}
			else
			{
				count += (node - iter.node - 1) * buf_size;
				count += cur - first;
				count += iter.last - iter.cur;
				return count;
			}
		}

		iterator& operator++() 
		{
			*this = *this + 1;
			return *this; 
		}
		iterator& operator--() 
		{
			*this = this->operator-(1);
			return *this; 
		}
		iterator operator++(int)
		{
			iterator tmp = *this;
			*this = *this + 1;
			return tmp;
		}
		iterator operator--(int)
		{
			iterator tmp = *this;
			*this = this->operator--();
			return tmp;
		}

		void clear() noexcept
		{
			cur = nullptr;
			first = nullptr;
			last = nullptr;
			node = nullptr;
		}
	};

	template<typename U, typename Ref = const U&, typename Ptr = const U*>
	struct deque_const_iterator
	{
		using iterator = deque_const_iterator<U, Ref, Ptr>;
		using iterator_category = std::random_access_iterator_tag;
		using value_type = U;
		using pointer = Ptr;
		using reference = Ref;
		using difference_type = std::ptrdiff_t;
		using map_pointer = U** ;
		using size_type = std::size_t;

		const U *cur;
		const U *first;          //当前缓冲区第一个元素
		const U *last;		//当前缓冲区最后元素的下一位置
		map_pointer node;
		size_type buf_size = sizeof(U) >= 128 ? 1 : 128 / sizeof(U);

		deque_const_iterator() : cur(nullptr), first(nullptr),
			last(nullptr), node(nullptr) {}
		
		deque_const_iterator(pointer cur, pointer first, pointer last,
			map_pointer node) : cur(cur), first(first),
			last(last), node(node) {}
		
		deque_const_iterator(deque_iterator<U> iter):
			cur(iter.cur), first(iter.first), last(iter.last),
			node(iter.node) {}
		
		deque_const_iterator& operator=(deque_iterator<U> iter)
		{
			cur = iter.cur;
			first = iter.first;
			last = iter.last;
			node = iter.node;
			return *this;
		}


		bool operator==(const iterator& iter) const noexcept
		{
			return cur == iter.cur;
		}
		bool operator!=(const iterator& iter) const noexcept
		{
			return cur != iter.cur;
		}
		bool operator<(const iterator& iter) const noexcept
		{
			return node == iter.node ? cur < iter.cur : node < cur.node;
		}
		bool operator>(const iterator& iter) const noexcept
		{
			return node == iter.node ? cur > iter.cur : node > cur.node;
		}

		reference operator*() const noexcept { return *cur; }
		pointer operator->() const noexcept { return this->cur; }

		template<typename I>
		friend deque_const_iterator<I>
		operator+(deque_const_iterator<I> iter, size_type n)
		{
			if (n < iter.last - iter.cur) {
				iter.cur += n;
			}
			else {
				size_type jump = 1 + (n - (iter.last - iter.cur)) / iter.buf_size;
				iter.node += jump;
				iter.cur = *iter.node + (n - (iter.last - iter.cur)) % iter.buf_size;
				iter.first = *iter.node;
				iter.last = *iter.node + iter.buf_size;
			}

			return iter;
		}

		iterator operator-(size_type n) const
		{
			iterator tmp = *this;

			if (n <= tmp.cur - tmp.first) {
				tmp.cur -= n;
			}
			else {
				size_type jump = 1 + (n - (tmp.cur - tmp.first + 1)) / buf_size;
				tmp.node -= jump;
				tmp.last = *tmp.node + tmp.buf_size;
				tmp.cur = tmp.last - 1 - (n - (tmp.cur - tmp.first + 1)) % buf_size;
				tmp.first = *tmp.node;
			}

			return tmp;
		}

		difference_type operator-(const iterator& iter) const
		{
			difference_type count = 0;
			if (node == iter.node)
			{
				count += cur - iter.cur;
				return count;
			}
			else
			{
				count += (node - iter.node - 1) * buf_size;
				count += cur - first;
				count += iter.last - iter.cur;
				return count;
			}
		}


		iterator& operator++()
		{
			*this = *this + 1;
			return *this;
		}
		iterator& operator--()
		{
			*this = this->operator-(1);
			return *this;
		}
		iterator operator++(int)
		{
			iterator tmp = *this;
			*this = *this + 1;
			return tmp;
		}
		iterator operator--(int)
		{
			iterator tmp = *this;
			*this = this->operator--();
			return tmp;
		}

		void clear() noexcept
		{
			cur = nullptr;
			first = nullptr;
			last = nullptr;
			node = nullptr;
		}
	};
	
	using size_type = std::size_t;
	using value_type = T;
	using pointer = T * ;
	using reference = T & ;
	using difference_type = std::ptrdiff_t;
	using iterator = deque_iterator<T>;
	using const_iterator = deque_const_iterator<T>;
	using const_reference = const reference;
	using allocator_type = Alloc;
	
protected:
	using map_pointer = pointer * ;

public:
	cx_deque();
	explicit cx_deque(size_type n, const value_type& value = value_type());
	explicit cx_deque(std::initializer_list<value_type> list);
	cx_deque(const cx_deque<T>& deq);
	cx_deque(cx_deque<T>&& deq) noexcept;
	cx_deque<T, Alloc>& operator=(const cx_deque<T>& deq);
	cx_deque<T, Alloc>& operator=(cx_deque<T>&& deq) noexcept;
	~cx_deque() noexcept;

	iterator begin() noexcept { return start; }
	iterator end() noexcept { return finish; }
	const_iterator cbegin() const noexcept { return start; }
	const_iterator cend() const noexcept { return finish; }
	
	reference operator[](size_type n) { return *(start + n); }
	const_reference operator[](size_type n) const { return *(start + n); }
	reference front() { return *start; }
	const_reference front() const { return *start; }
	reference back() { return *(finish - 1); }
	const_reference back() const { return *(finish - 1); }
	
	void swap(cx_deque<T>& deq) noexcept;
	template<typename U>
	friend void swap(cx_deque<U>& ls, cx_deque<U>& rs) { ls.swap(rs); }

	size_type size() const noexcept { return finish - start; };
	bool empty() const noexcept { return start == finish; }
	void clear() noexcept;

	void push_back(const value_type& val);
	void push_back(value_type&& val);
	void push_front(const value_type& val);
	void push_front(value_type&& val);
	void pop_back();
	void pop_front();

	iterator erase(iterator pos);
	iterator erase(iterator beg, iterator end);

	iterator insert(iterator pos, const value_type& val);
	iterator insert(iterator pos, value_type&& val);

	friend bool operator==<>(const cx_deque& lhs, 
							 const cx_deque& rhs);
	friend bool operator!=<>(const cx_deque& lhs,
							 const cx_deque& rhs);


protected:
	void create_map(size_type element_num);
	void reallocate_map(size_type node_to_add, bool add_at_front);


protected:
	iterator start;
	iterator finish;

	map_pointer map;
	size_type map_size = 4;
	size_type buf_size = sizeof(T) >= 128 ? 1 : 128 / sizeof(T);
};


template<typename T, typename Alloc>
cx_deque<T, Alloc>::cx_deque()
{
	size_type init_num = 16;
	size_type node_num = init_num / buf_size + 1;
	map_size = std::max(map_size, node_num + 2);
	map = free_list_allocator<pointer>::allocate(map_size);

	map_pointer start_ptr = map + (map_size - node_num) / 2;
	*start_ptr = Alloc::allocate(buf_size);

	start.node = start_ptr;
	start.cur = *(start.node);
	start.first = start.cur;
	start.last = start.first + buf_size;

	finish = start;
}

template<typename T, typename Alloc>
cx_deque<T, Alloc>::cx_deque(size_type n, const value_type& value)
{
	create_map(n);
	finish = std::uninitialized_fill_n(start, n, value);
}

template<typename T, typename Alloc>
cx_deque<T, Alloc>::cx_deque(std::initializer_list<T> list)
{
	create_map(list.size());
	finish = std::uninitialized_move(list.begin(), list.end(), start);
}

template<typename T, typename Alloc>
cx_deque<T, Alloc>::cx_deque(const cx_deque<T>& deq)
{
	create_map(deq.size());
	finish = std::uninitialized_copy(deq.cbegin(), deq.cend(), start);
}

template<typename T, typename Alloc>
cx_deque<T, Alloc>::cx_deque(cx_deque<T>&& deq) noexcept
{
	start = deq.start;
	finish = deq.finish;
	map = deq.map;
	
	deq.start.clear();
	deq.finish.clear();
	deq.map = nullptr;
}

template<typename T, typename Alloc>
cx_deque<T, Alloc>&
cx_deque<T, Alloc>::operator=(cx_deque<T>&& deq) noexcept
{
	swap(deq);
	return *this;
}

template<typename T, typename Alloc>
cx_deque<T, Alloc>& 
cx_deque<T, Alloc>::operator=(const cx_deque<T>& deq)
{
	cx_deque<T> tmp(deq);
	swap(tmp);
	return *this;
}


template<typename T, typename Alloc>
cx_deque<T, Alloc>::~cx_deque() noexcept
{
	if (!map)
		return;

	alloc::destroy(start, finish);

	for (map_pointer map_ptr = start.node; map_ptr <= finish.node; 
		 ++map_ptr)
	{
		Alloc::deallocate(*map_ptr, buf_size);
	}

	free_list_allocator<pointer>::deallocate(map, map_size);
}


template<typename T, typename Alloc>
void cx_deque<T, Alloc>::create_map(size_type element_num)
{
	size_type node_num = element_num / buf_size + 1;
	map_size = std::max(map_size, node_num + 2);
	map = free_list_allocator<pointer>::allocate(map_size);
	
	map_pointer start_ptr, finish_ptr;
	start_ptr = map + (map_size - node_num) / 2;
	finish_ptr = start_ptr + node_num - 1;
	for (map_pointer ptr = start_ptr; ptr <= finish_ptr; ++ptr)
	{
		*ptr = Alloc::allocate(buf_size);
	}

	start.node = start_ptr;
	start.cur = *(start.node); 
	start.first = start.cur; 
	start.last = start.first + buf_size;
	
	finish = start;
}


template<typename T, typename Alloc>
void cx_deque<T, Alloc>::reallocate_map(size_type node_to_add,
							bool add_at_front)
{
	size_type old_node_num = finish.node - start.node + 1;
	size_type new_node_num = old_node_num + node_to_add;
	map_pointer start_ptr, finish_ptr;
	
	if (map_size > 2 * new_node_num)
	{
		start_ptr = map + (map_size - new_node_num) / 2;
		finish_ptr = start_ptr + new_node_num - 1;

		if (!add_at_front) {
			std::copy_n(start.node, old_node_num, start_ptr);
			start.node = start_ptr;
			finish.node = start.node + old_node_num - 1;
		}
		else {
			start.node = std::copy_backward(start.node, finish.node + 1, finish_ptr + 1);
			finish.node = finish_ptr;
		}
	}
	else
	{
		size_type old_map_size = map_size;
		map_size = map_size + std::max(map_size, node_to_add) + 2;
		map_pointer new_map = free_list_allocator<pointer>::allocate(map_size);
	
		start_ptr = new_map + (map_size - new_node_num) / 2;
		finish_ptr = start_ptr + new_node_num - 1;

		if (!add_at_front) {
			std::copy_n(start.node, old_node_num, start_ptr);
			start.node = start_ptr;
			finish.node = start.node + old_node_num - 1;
		}
		else {
			start.node = std::copy_backward(start.node, finish.node + 1, finish_ptr + 1);
			finish.node = finish_ptr;
		}
		
		free_list_allocator<pointer>::deallocate(map, old_map_size);
		map = new_map;
	}
}


template<typename T, typename Alloc>
void cx_deque<T, Alloc>::swap(cx_deque<T>& deq) noexcept
{
	std::swap(start, deq.start);
	std::swap(finish, deq.finish);
	std::swap(map, deq.map);	
}


template<typename T, typename Alloc>
void cx_deque<T, Alloc>::clear() noexcept
{
	alloc::destroy(start, finish);

	for (map_pointer node = start.node + 1; node <= finish.node; ++node)
	{
		Alloc::deallocate(*node, buf_size);
	}

	start.cur = start.first;
	finish = start;
}



template<typename T, typename Alloc>
void cx_deque<T, Alloc>::push_back(const value_type& val)
{
	if (finish.cur < finish.last - 1)
	{
		alloc::construct(finish.cur, val);
		++finish;
	}
	else 
	{
		if (finish.node == map + map_size - 1) {
			reallocate_map(1, false);
		}

		map_pointer next_node = finish.node + 1;
		*next_node = Alloc::allocate(buf_size);
		alloc::construct(finish.cur, val);
		++finish;
	}
}


template<typename T, typename Alloc>
void cx_deque<T, Alloc>::push_back(value_type&& val)
{
	if (finish.cur < finish.last - 1)
	{
		alloc::construct(finish.cur, std::forward<T>(val));
		++finish;
	}
	else
	{
		if (finish.node == map + map_size - 1) {
			reallocate_map(1, false);
		}

		map_pointer next_node = finish.node + 1;
		*next_node = Alloc::allocate(buf_size);
		alloc::construct(finish.cur, std::forward<T>(val));
		++finish;
	}
}


template<typename T, typename Alloc>
void cx_deque<T, Alloc>::push_front(const value_type& val)
{
	if (start.cur > start.first)
	{
		--start;
		alloc::construct(start.cur, val);
	}
	else
	{
		if (start.node == map) {
			reallocate_map(1, true);
		}

		map_pointer prev_node = start.node - 1;
		*prev_node = Alloc::allocate(buf_size);
		--start;
		alloc::construct(start.cur, val);
	}
}


template<typename T, typename Alloc>
void cx_deque<T, Alloc>::push_front(value_type&& val)
{
	if (start.cur > start.first)
	{
		--start;
		alloc::construct(start.cur, std::forward<value_type>(val));
	}
	else
	{
		if (start.node == map) {
			reallocate_map(1, true);
		}

		map_pointer prev_node = start.node - 1;
		*prev_node = Alloc::allocate(buf_size);
		--start;
		alloc::construct(start.cur, std::forward<value_type>(val));
	}
}



template<typename T, typename Alloc>
void cx_deque<T, Alloc>::pop_back()
{
	--finish;
	if(finish.cur == finish.last - 1)
		Alloc::deallocate(*(finish.node + 1), buf_size);
	alloc::destroy(finish.cur);
}


template<typename T, typename Alloc>
void cx_deque<T, Alloc>::pop_front()
{
	alloc::destroy(start.cur);
	++start;
	if (start.cur == start.first)
		Alloc::deallocate(*(start.node - 1), buf_size);
}


template<typename T, typename Alloc>
typename cx_deque<T, Alloc>::iterator
cx_deque<T, Alloc>::erase(iterator pos)
{
	return erase(pos, pos + 1);
}


template<typename T, typename Alloc>
typename cx_deque<T, Alloc>::iterator
cx_deque<T, Alloc>::erase(iterator beg, iterator end)
{
	if (finish - end < beg - start)
	{
		iterator old_finish = finish;
		finish = std::move(end, finish, beg);

		alloc::destroy(finish, old_finish);
		for (map_pointer node = finish.node + 1; node <= old_finish.node;
			 ++node) {
			Alloc::deallocate(*node, buf_size);
		}

		return beg;
	}
	else
	{
		iterator old_start = start;
		start = std::move_backward(start, beg, end);
		 
		alloc::destroy(old_start, start);
		for (map_pointer node = old_start.node; node != start.node;
			 ++node){
			Alloc::deallocate(*node, buf_size);
		}

		return end;
	}
}


template<typename T, typename Alloc>
typename cx_deque<T, Alloc>::iterator
cx_deque<T, Alloc>::insert(iterator pos, const value_type& val)
{
	if (pos == finish) {
		push_back(val);
		return finish - 1;
	}
	if (pos == start){
		push_front(val);
		return start;
	}

	if (finish - pos < pos - start)
	{
		push_back(back());
		std::move_backward(pos, finish - 2, finish - 1);
		*pos = val;
		return pos;
	}
	else
	{
		push_front(front());
		std::move(start + 2, pos + 1, start + 1);
		*(pos) = val;
		return pos;
	}
}


template<typename T, typename Alloc>
typename cx_deque<T, Alloc>::iterator
cx_deque<T, Alloc>::insert(iterator pos, value_type&& val)
{
	if (pos == finish) {
		push_back(std::forward<value_type>(val));
		return finish - 1;
	}
	if (pos == start) {
		push_front(std::forward<value_type>(val));
		return start;
	}

	if (finish - pos < pos - start)
	{
		push_back(back());
		std::move_backward(pos, finish - 2, finish - 1);
		*pos = std::forward<value_type>(val);
		return pos;
	}
	else
	{
		push_front(front());
		std::move(start + 2, pos + 1, start + 1);
		*pos = std::forward<value_type>(val);
		return pos;
	}
}



template<typename T, typename Alloc>
bool operator==(const cx_deque<T, Alloc>& lhs,
				const cx_deque<T, Alloc>& rhs)
{
	if (lhs.size() != rhs.size()) {
		return false;
	}

	for (std::size_t i = 0; i < lhs.size(); ++i) {
		if (lhs[i] != rhs[i])
			return false;
	}

	return true;
}

template<typename T, typename Alloc>
bool operator!=(const cx_deque<T, Alloc>& lhs,
				const cx_deque<T, Alloc>& rhs)
{
	return !(lhs == rhs);
}












