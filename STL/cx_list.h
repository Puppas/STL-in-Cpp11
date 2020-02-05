#pragma once
#include <iterator>
#include "free_list_allocator.h"
#include "alloc_destroy.h"
#include <initializer_list>


template<typename T>
struct list_node
{
	T data;
	list_node<T> *next;
	list_node<T> *prev;
};


template<typename T, typename Alloc = free_list_allocator<list_node<T>>>
class cx_list
{
protected:
	template<typename U, typename Ref = U&, typename Ptr = list_node<U>*>
	struct list_iterator;

	template<typename U, typename Ref = const U & , typename Ptr = const list_node<U>*>
	struct list_const_iterator
	{
		const list_node<U> *node_ptr;

		using iterator = list_const_iterator<U, Ref, Ptr>;
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = U;
		using reference = Ref;
		using pointer = Ptr;
		using difference_type = std::ptrdiff_t;

		list_const_iterator() : node_ptr(nullptr) {}
		list_const_iterator(list_node<U> *ptr) : node_ptr(ptr) {}
		list_const_iterator(const list_const_iterator<U>& iter) :
			node_ptr(iter.node_ptr) {}
		list_const_iterator(const list_iterator<U>& iter):
			node_ptr(iter.node_ptr) {}
		iterator& operator=(const list_const_iterator<U>& iter) {
			node_ptr = iter.node_ptr;
			return *this;
		}
		iterator& operator=(const list_iterator<U>& iter) {
			node_ptr = iter.node_ptr;
			return *this;
		}

		bool operator==(const iterator& iter) const
		{
			return node_ptr == iter.node_ptr;
		}
		bool operator!=(const iterator& iter) const
		{
			return node_ptr != iter.node_ptr;
		}

		reference operator*() const { return node_ptr->data; }
		pointer operator->() const { return node_ptr; }

		iterator& operator++()
		{
			node_ptr = node_ptr->next;
			return *this;
		}
		iterator operator++(int)
		{
			iterator tmp = *this;
			node_ptr = node_ptr->next;
			return tmp;
		}
		iterator& operator--()
		{
			node_ptr = node_ptr->prev;
			return *this;
		}
		iterator operator--(int)
		{
			iterator tmp = *this;
			node_ptr = node_ptr->prev;
			return tmp;
		}
	};

	template<typename U, typename Ref, typename Ptr>
	struct list_iterator
	{
		list_node<T> *node_ptr;

		using iterator = list_iterator<U, Ref, Ptr>;
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = U;
		using reference = Ref;
		using pointer = Ptr;
		using difference_type = std::ptrdiff_t;
		
		list_iterator(): node_ptr(nullptr) {}
		list_iterator(list_node<U> *ptr): node_ptr(ptr) {}
		list_iterator(const list_iterator<U>& iter): 
						node_ptr(iter.node_ptr) {}

		iterator& operator=(const list_iterator<U>& iter) {
			node_ptr = iter.node_ptr;
			return *this;
		}

		bool operator==(const iterator& iter) const 
		{
			return this->node_ptr == iter.node_ptr;
		}
		bool operator!=(const iterator& iter) const
		{
			return this->node_ptr != iter.node_ptr;
		}

		reference operator*() const { return node_ptr->data; }
		pointer operator->() const { return node_ptr; }

		iterator& operator++()
		{
			node_ptr = node_ptr->next;
			return *this;
		}
		iterator operator++(int)
		{
			iterator tmp = *this;
			node_ptr = node_ptr->next;
			return tmp;
		}
		iterator& operator--()
		{
			node_ptr = node_ptr->prev;
			return *this;
		}
		iterator operator--(int)
		{
			iterator tmp = *this;
			node_ptr = node_ptr->prev;
			return tmp;
		}
	};

public:
	using value_type = T;
	using pointer = value_type * ;
	using iterator = list_iterator<T> ;
	using const_iterator = list_const_iterator<T>;
	using reference = value_type & ;
	using const_reference = const reference;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;


public:
	cx_list();
	explicit cx_list(std::initializer_list<T> init_val);
	cx_list(const cx_list<T>& list);
	cx_list(cx_list<T>&& list);
	cx_list<T, Alloc>& operator=(const cx_list<T>& list);
	~cx_list();

	iterator begin() const { return iterator(last_iter->next); }
	const_iterator cbegin() const { return const_iterator(last_iter->next); }
	iterator end() const { return last_iter; }
	const_iterator cend() const { return const_iterator(last_iter); }
	bool empty() const { return iterator(last_iter->next) == last_iter; }
	size_type size() const;
	reference front() { return *begin(); }
	const_reference front() const { return *begin(); }
	reference back() { return *(--end()); }
	const_reference back() const { return *(--end()); }

	iterator insert(iterator pos, const T& val);
	void push_front(const T& val) { insert(begin(), val); }
	void push_back(const T& val) { insert(end(), val); }

	iterator erase(iterator pos) {
		iterator next_node(pos->next);
		destroy_node(pos);
		return next_node;
	}
	void pop_front() { erase(begin()); }
	void pop_back() {
		erase(iterator(last_iter->prev));
	}

	void clear();
	void remove(const T& val);
	void unique();

	void swap(cx_list<T>& list) {
		std::swap(last_iter, list.last_iter);
	}
	template<typename U>
	friend void swap(cx_list<U>& ls, cx_list<U>& rs) {
		ls.swap(rs);
	}
	
	void splice(iterator pos, cx_list<T>& list) {
		transfer(pos, list.begin(), list.end());
	}
	void splice(iterator pos, cx_list<T>& list, iterator iter) {
		transfer(pos, iter, iterator(iter->next));
	}
	void splice(iterator pos, cx_list<T>& list,
			    iterator beg, iterator end) {
		transfer(pos, beg, end);
	}

	void merge(cx_list<T>& list);      //已排序下将元素转移至类中
	void reverse();
	void sort();

	friend bool operator==<>(const cx_list& lhs,
							 const cx_list& rhs);

	friend bool operator!=<>(const cx_list& lhs,
							 const cx_list& rhs);


protected:
	iterator last_iter;     //指向尾端的空白节点

	iterator create_node();
	iterator create_node(const T& val);
	void destroy_node(iterator& iter);      //不能删除last_iter
	void transfer(iterator pos, iterator first, iterator last);
};


template<typename T, typename Alloc>
cx_list<T, Alloc>::cx_list()
{
	last_iter = create_node();
	last_iter->next = last_iter.node_ptr;
	last_iter->prev = last_iter.node_ptr;
}


template<typename T, typename Alloc>
cx_list<T, Alloc>::cx_list(std::initializer_list<T> init_val)
{
	last_iter = create_node();
	last_iter->next = last_iter.node_ptr;
	last_iter->prev = last_iter.node_ptr;

	for (auto iter = init_val.begin(); iter != init_val.end(); ++iter)
		push_back(*iter);
}


template<typename T, typename Alloc>
cx_list<T, Alloc>::cx_list(const cx_list<T>& list)
{
	last_iter = create_node();
	last_iter->next = last_iter.node_ptr;
	last_iter->prev = last_iter.node_ptr;

	for (auto iter = list.cbegin(); iter != list.cend(); ++iter)
	{
		push_back(*iter);
	}
}

template<typename T, typename Alloc>
cx_list<T, Alloc>::cx_list(cx_list<T>&& list)
{
	last_iter = create_node();
	last_iter->next = last_iter.node_ptr;
	last_iter->prev = last_iter.node_ptr;

	transfer(begin(), list.begin(), list.end());
}


template<typename T, typename Alloc>
cx_list<T, Alloc>&
cx_list<T, Alloc>::operator=(const cx_list<T>& list)
{
	cx_list<T> new_list(list);
	swap(new_list);
	return *this;
}



template<typename T, typename Alloc>
cx_list<T, Alloc>::~cx_list()
{
	iterator old_iter;
	for (iterator iter = begin(); iter != end(); )
	{
		old_iter = iter;
		alloc::destroy(&(iter->data));
		++iter;
		Alloc::deallocate(old_iter.node_ptr, 1);
	}

	Alloc::deallocate(last_iter.node_ptr, 1);
}

template<typename T, typename Alloc>
typename cx_list<T, Alloc>::size_type
cx_list<T, Alloc>::size() const
{
	std::size_t n = 0;
	for (const_iterator iter = cbegin(); iter != cend(); ++iter)
		++n;

	return n;
}


template<typename T, typename Alloc>
typename cx_list<T, Alloc>::iterator 
cx_list<T, Alloc>::insert(iterator pos, const T & val)
{
	iterator new_node = create_node(val);
	iterator prev_node(pos->prev);

	prev_node->next = new_node.node_ptr;
	new_node->prev = prev_node.node_ptr;
	new_node->next = pos.node_ptr;
	pos->prev = new_node.node_ptr;

	return new_node;
}


template<typename T, typename Alloc>
void cx_list<T, Alloc>::clear()
{
	iterator old_iter;
	for (iterator iter = begin(); iter != end(); )
	{
		old_iter = iter;
		alloc::destroy(&(iter->data));
		++iter;
		Alloc::deallocate(old_iter.node_ptr, 1);
	}

	last_iter->next = last_iter.node_ptr;
	last_iter->prev = last_iter.node_ptr;
}


template<typename T, typename Alloc>
void cx_list<T, Alloc>::remove(const T& val)
{
	for (iterator iter = begin(); iter != end(); )
	{
		if (*iter == val) {
			iter = erase(iter);
			continue;
		}
		++iter;
	}
}


template<typename T, typename Alloc>
void cx_list<T, Alloc>::unique()
{
	for (iterator prev = begin(), next = std::next(begin());
		 next != end();)
	{
		if (*prev == *next) {
			next = erase(next);
			continue;
		}

		++prev;
		++next;
	}
}


template<typename T, typename Alloc>
void cx_list<T, Alloc>::merge(cx_list<T>& list)
{
	iterator iter1, iter2;

	for (iter1 = begin(), iter2 = list.begin();
		 iter1 != end() && iter2 != list.end(); )
	{
		if (*iter2 <= *iter1)
		{
			iterator next(iter2->next);
			transfer(iter1, iter2, next);
			iter2 = next;
		}
		else
		{
			++iter1;
		}
	}

	transfer(end(), iter2, list.end());
}


template<typename T, typename Alloc>
void cx_list<T, Alloc>::reverse()
{
	for (auto iter = begin(); iter != end(); --iter)
	{
		std::swap(iter->next, iter->prev);
	}

	std::swap(last_iter->next, last_iter->prev);
}


template<typename T, typename Alloc>
void cx_list<T, Alloc>::sort()
{
	if (size() == 0 || size() == 1)
		return;

	cx_list<T, Alloc> carry;
	const std::size_t COUNTER_SIZE = 64;
	cx_list<T, Alloc> counter[COUNTER_SIZE];
	int i, limit = -1;

	while (!empty())
	{
		i = 0;

		if (counter[i].empty()) {
			splice(counter[i].begin(), *this, begin());
		}
		else {
			carry.splice(carry.begin(), *this, begin());

			while (i < COUNTER_SIZE && !counter[i].empty()) {
				carry.merge(counter[i]);
				++i;
			}
			counter[i].splice(counter[i].begin(), carry);
		}

		limit = std::max(i, limit);
	}

	for (i = 1; i <= limit; ++i){
		counter[i].merge(counter[i - 1]);
	}

	swap(counter[limit]);
}



template<typename T, typename Alloc>
typename cx_list<T, Alloc>::iterator
cx_list<T, Alloc>::create_node()
{
	list_node<T> *ptr = Alloc::allocate(1);
	ptr->next = nullptr;
	ptr->prev = nullptr;

	return iterator(ptr);
}



template<typename T, typename Alloc>
typename cx_list<T, Alloc>::iterator
cx_list<T, Alloc>::create_node(const T& val)
{
	list_node<T> *ptr = Alloc::allocate(1);
	alloc::construct(&(ptr->data), val);
	ptr->next = nullptr;
	ptr->prev = nullptr;

	return iterator(ptr);
}


template<typename T, typename Alloc>
void cx_list<T, Alloc>::destroy_node(
		typename cx_list<T, Alloc>::iterator& iter)
{
	iterator prev_node(iter->prev);
	iterator next_node(iter->next);
	prev_node->next = next_node.node_ptr;
	next_node->prev = prev_node.node_ptr;

	alloc::destroy(&(iter->data));
	Alloc::deallocate(iter.node_ptr, 1);
}



//将[first, last)的元素移动到pos前
template<typename T, typename Alloc>
void cx_list<T, Alloc>::transfer(
		typename cx_list<T, Alloc>::iterator pos,
		typename cx_list<T, Alloc>::iterator first,
		typename cx_list<T, Alloc>::iterator last)
{
	if (pos != last && first != last){
		iterator prev_pos(pos->prev);
		iterator prev_first(first->prev);
		iterator prev_last(last->prev);

		prev_first->next = last.node_ptr;
		last->prev = prev_first.node_ptr;

		prev_pos->next = first.node_ptr;
		first->prev = prev_pos.node_ptr;
		prev_last->next = pos.node_ptr;
		pos->prev = prev_last.node_ptr;
	}
}


template<typename T, typename Alloc>
bool operator==(const cx_list<T, Alloc>& lhs,
				const cx_list<T, Alloc>& rhs)
{
	if (lhs.size() != rhs.size())
		return false;

	for (auto l_iter = lhs.cbegin(), r_iter = rhs.cbegin(); 
		 l_iter != lhs.cend(); 
		 ++l_iter, ++r_iter) {

		if (l_iter->data != r_iter->data) {
			return false;
		}
	}

	return true;
}


template<typename T, typename Alloc>
bool operator!=(const cx_list<T, Alloc>& lhs,
				const cx_list<T, Alloc>& rhs)
{
	return !(lhs == rhs);
}