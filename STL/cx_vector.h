#pragma once
#include "free_list_allocator.h"
#include <memory>
#include "alloc_destroy.h"
#include <algorithm>
#include <initializer_list>


template<typename T, typename Alloc = free_list_allocator<T>>
class cx_vector
{
public:
	using value_type = T;
	using pointer = value_type * ;
	using iterator = value_type * ;
	using const_iterator = const value_type *;
	using reference = value_type & ;
	using const_reference = const value_type&;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;


protected:
	iterator start;    //目前使用空间的头
	iterator finish;   //目前使用空间的尾
	iterator end_of_storage;        //目前可用空间的尾

	void fill_initialize(size_type n, const T& value);
	
public:
	iterator begin() const { return start; }
	iterator end() const { return finish; }
	const_iterator cbegin() const { return start; }
	const_iterator cend() const { return finish; }

	size_type size() const { return finish - start; }
	size_type capacity() const { return end_of_storage - start; }
	bool empty() const { return start == finish; }
	reference operator[](size_type n) { return *(start + n); }
	const_reference operator[](size_type n) { return *(start + n); }

	cx_vector();
	cx_vector(size_type n, const T& value) { fill_initialize(n, value); }
	cx_vector(int n, const T& value) { fill_initialize(n, value); }
	cx_vector(long n, const T& value) { fill_initialize(n, value); }
	explicit cx_vector(std::initializer_list<T> list);
	cx_vector(const cx_vector<T>& vec);
	cx_vector(cx_vector<T>&& vec);
	explicit cx_vector(size_type n) { fill_initialize(n, T()); }
	cx_vector<T, Alloc>& operator=(const cx_vector<T>& vec);
	cx_vector<T, Alloc>& operator=(cx_vector<T>&& vec);

	void swap(cx_vector<T>& vec);
	template<typename U>
	friend void swap(cx_vector<U>& ls, cx_vector<U>& rs)
	{
		ls.swap(rs);
	}

	~cx_vector() {
		destroy(start, finish);
		Alloc::deallocate(start, end_of_storage - start);
	}

	reference front() { return *start; }
	const_reference front() const { return *start; }
	reference back() { return *(finish - 1); }
	const_reference back() const { return *(finish - 1); }

	void push_back(const T& value);
	void pop_back();
	iterator erase(iterator pos);
	iterator erase(iterator beg, iterator end);
	void clear() { erase(start, finish); }

	iterator insert(iterator pos, size_type n, const T& value);

	friend bool operator==<>(const cx_vector& lhs, 
							 const cx_vector& rhs);

	friend bool operator!=<>(const cx_vector& lhs,
							 const cx_vector& rhs);
};


template<typename T, typename Alloc>
cx_vector<T, Alloc>::cx_vector()
{
	start = Alloc::allocate(16);
	finish = start;
	end_of_storage = start + 16;
}


template<typename T, typename Alloc>
void cx_vector<T, Alloc>::fill_initialize(
			typename cx_vector<T, Alloc>::size_type n,
			const T& value)
{
	start = Alloc::allocate(n);
	finish = std::uninitialized_fill_n(start, n, value);
	end_of_storage = finish;
}


template<typename T, typename Alloc>
cx_vector<T, Alloc>::cx_vector(std::initializer_list<T> list)
{
	start = Alloc::allocate(list.size());
	finish = std::uninitialized_copy(list.begin(), list.end(), start);
	end_of_storage = finish;
}


template<typename T, typename Alloc>
cx_vector<T, Alloc>::cx_vector(const cx_vector<T>& vec)
{
	start = Alloc::allocate(vec.size());
	finish = std::uninitialized_copy(vec.cbegin(), vec.cend(), start);
	end_of_storage = finish;
}


template<typename T, typename Alloc>
cx_vector<T, Alloc>::cx_vector(cx_vector<T>&& vec)
{
	start = vec.start;
	finish = vec.finish;
	end_of_storage = vec.end_of_storage;

	vec.start = nullptr;
	vec.finish = nullptr;
	vec.end_of_storage = nullptr;
}


template<typename T, typename Alloc>
void cx_vector<T, Alloc>::swap(cx_vector<T>& vec)
{
	using std::swap;
	swap(start, vec.start);
	swap(finish, vec.finish);
	swap(end_of_storage, vec.end_of_storage);
}


template<typename T, typename Alloc>
cx_vector<T, Alloc>& 
cx_vector<T, Alloc>::operator=(const cx_vector<T>& vec)
{
	cx_vector<T, Alloc> new_vec(vec);
	swap(new_vec);
	return *this;
}

template<typename T, typename Alloc>
cx_vector<T, Alloc>&
cx_vector<T, Alloc>::operator=(cx_vector<T>&& vec)
{
	swap(vec);
	return *this;
}



template<typename T, typename Alloc>
void cx_vector<T, Alloc>::push_back(const T& value)
{
	if (finish != end_of_storage)
	{
		construct(finish, value);
		++finish;
	}
	else
	{
		size_type old_size = size();
		size_type new_size = old_size == 0 ? 1 : 2 * old_size;

		iterator new_start = Alloc::allocate(new_size);
		iterator new_finish = std::_Uninitialized_move_unchecked(start, finish, new_start);
		construct(new_finish, value);
		++new_finish;

		destroy(start, finish);
		Alloc::deallocate(start, old_size);
		
		start = new_start;
		finish = new_finish;
		end_of_storage = new_start + new_size;
	}
}


template<typename T, typename Alloc>
void cx_vector<T, Alloc>::pop_back()
{
	--finish;
	destroy(finish);
}


template<typename T, typename Alloc>
typename cx_vector<T, Alloc>::iterator
cx_vector<T, Alloc>::erase(typename cx_vector<T, Alloc>::iterator pos)
{
	if (pos != finish - 1)
	{
		std::move(pos + 1, finish, pos);
	}

	--finish;
	destroy(finish);
	return pos;
}



template<typename T, typename Alloc>
typename cx_vector<T, Alloc>::iterator
cx_vector<T, Alloc>::erase(typename cx_vector<T, Alloc>::iterator beg,
						   typename cx_vector<T, Alloc>::iterator end)
{
	if (end != finish)
	{
		std::move(end, finish, beg);
	}

	destroy(finish - end + beg, finish);
	finish -= end - beg;
	return beg;
}
 

template<typename T, typename Alloc>
typename cx_vector<T, Alloc>::iterator
cx_vector<T, Alloc>::insert(typename cx_vector<T, Alloc>::iterator pos,
							typename cx_vector<T, Alloc>::size_type n,
							const T& value)
{
	if (end_of_storage - finish >= n)
	{
		size_type elem_after = finish - pos;

		if (elem_after > n) {
			std::_Uninitialized_move_unchecked(finish - n, finish, finish);
			std::move(pos, finish - n, pos + n);
			std::fill_n(pos, n, value);
			finish += n;
		}
		else {
			iterator old_finish = finish;
			finish = std::uninitialized_fill_n(finish, n - elem_after, value);
			finish = std::_Uninitialized_move_unchecked(pos, old_finish, finish);
			std::fill_n(pos, elem_after, value);
		}
	}
	else
	{
		size_type old_size = size();
		size_type new_size = old_size + std::max(old_size, n);

		iterator new_start = Alloc::allocate(new_size);
		iterator new_finish = std::_Uninitialized_move_unchecked(start, pos, new_start);
		new_finish = std::uninitialized_fill_n(new_finish, n, value);
		new_finish = std::_Uninitialized_move_unchecked(pos, finish, new_finish);
		
		destroy(start, finish);
		Alloc::deallocate(start, old_size);

		start = new_start;
		finish = new_finish;
		end_of_storage = start + new_size;
	}

	return pos;
}


template<typename T, typename Alloc>
bool operator==(const cx_vector<T, Alloc>& lhs, 
				const cx_vector<T, Alloc>& rhs)
{
	if (lhs.size() != rhs.size())
		return false;

	for (std::size_t i = 0; i < lhs.size(); ++i) {
		if (lhs[i] != rhs[i])
			return false;
	}

	return true;
}


template<typename T, typename Alloc>
bool operator!=(const cx_vector<T, Alloc>& lhs,
				const cx_vector<T, Alloc>& rhs)
{
	return !(lhs == rhs);
}







