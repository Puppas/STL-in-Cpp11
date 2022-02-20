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
	using difference_type = std::ptrdiff_t;
	using size_type = std::size_t;
	using allocator_type = Alloc;

	static constexpr std::size_t INIT_SIZE = 16;

protected:
	iterator start;    //目前使用空间的头
	iterator finish;   //目前使用空间的尾
	iterator end_of_storage;        //目前可用空间的尾

	void fill_initialize(size_type n, const T& value);
	
public:
	iterator begin() noexcept { return start; }
	iterator end() noexcept { return finish; }
	const_iterator cbegin() const noexcept { return start; }
	const_iterator cend() const noexcept { return finish; }

	size_type size() const noexcept { return finish - start; }
	size_type capacity() const noexcept { return end_of_storage - start; }
	bool empty() const noexcept { return start == finish; }
	reference operator[](size_type n) { return *(start + n); }
	const_reference operator[](size_type n) const { return *(start + n); }

	cx_vector();
	cx_vector(size_type n, const T& value) { fill_initialize(n, value); }
	explicit cx_vector(std::initializer_list<T> list);
	cx_vector(const cx_vector<T>& vec);
	cx_vector(cx_vector<T>&& vec) noexcept;
	explicit cx_vector(size_type n) { fill_initialize(n, T()); }
	cx_vector<T, Alloc>& operator=(const cx_vector<T>& vec);
	cx_vector<T, Alloc>& operator=(cx_vector<T>&& vec) noexcept;

	void swap(cx_vector<T>& vec) noexcept;
	template<typename U>
	friend void swap(cx_vector<U>& ls, cx_vector<U>& rs)
	{
		ls.swap(rs);
	}

	~cx_vector() noexcept{
		if (!start)
			return;
		alloc::destroy(start, finish);
		Alloc::deallocate(start, end_of_storage - start);
	}

	reference front() { return *start; }
	const_reference front() const { return *start; }
	reference back() { return *(finish - 1); }
	const_reference back() const { return *(finish - 1); }

	void push_back(const T& val) { insert(finish, val); }
	void push_back(T&& val) { insert(finish, std::forward<T>(val)); }
	void pop_back();
	iterator erase(iterator pos);
	iterator erase(iterator beg, iterator end);
	void clear() noexcept { erase(start, finish); }

	iterator insert(iterator pos, size_type n, const T& value);
	iterator insert(iterator pos, const T& val) { return insert(pos, 1, val); }
	iterator insert(iterator pos, T&& val);


	friend bool operator==<>(const cx_vector& lhs, 
							 const cx_vector& rhs);

	friend bool operator!=<>(const cx_vector& lhs,
							 const cx_vector& rhs);
};


template<typename T, typename Alloc>
cx_vector<T, Alloc>::cx_vector()
{
	start = Alloc::allocate(INIT_SIZE);
	finish = start;
	end_of_storage = start + INIT_SIZE;
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
	finish = std::uninitialized_move(list.begin(), list.end(), start);
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
cx_vector<T, Alloc>::cx_vector(cx_vector<T>&& vec) noexcept
{
	start = vec.start;
	finish = vec.finish;
	end_of_storage = vec.end_of_storage;

	vec.start = nullptr;
	vec.finish = nullptr;
	vec.end_of_storage = nullptr;
}


template<typename T, typename Alloc>
void cx_vector<T, Alloc>::swap(cx_vector<T>& vec) noexcept
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
cx_vector<T, Alloc>::operator=(cx_vector<T>&& vec) noexcept
{
	swap(vec);
	return *this;
}


template<typename T, typename Alloc>
void cx_vector<T, Alloc>::pop_back()
{
	--finish;
	alloc::destroy(finish);
}


template<typename T, typename Alloc>
typename cx_vector<T, Alloc>::iterator
cx_vector<T, Alloc>::erase(typename cx_vector<T, Alloc>::iterator pos)
{
	return erase(pos, pos + 1);
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

	alloc::destroy(finish - end + beg, finish);
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
			std::uninitialized_move(finish - n, finish, finish);
			std::move_backward(pos, finish - n, finish);
			std::fill_n(pos, n, value);
			finish += n;
		}
		else {
			iterator old_finish = finish;
			finish = std::uninitialized_fill_n(finish, n - elem_after, value);
			finish = std::uninitialized_move(pos, old_finish, finish);
			std::fill_n(pos, elem_after, value);
		}
	}
	else
	{
		size_type old_size = size();
		size_type new_size = old_size + std::max(old_size, n);

		iterator new_start = Alloc::allocate(new_size);
		iterator new_finish = std::uninitialized_move(start, pos, new_start);
		new_finish = std::uninitialized_fill_n(new_finish, n, value);
		new_finish = std::uninitialized_move(pos, finish, new_finish);
		
		alloc::destroy(start, finish);
		Alloc::deallocate(start, old_size);

		start = new_start;
		finish = new_finish;
		end_of_storage = start + new_size;
	}

	return pos;
}


template<typename T, typename Alloc>
typename cx_vector<T, Alloc>::iterator
cx_vector<T, Alloc>::insert(typename cx_vector<T, Alloc>::iterator pos, T&& val)
{
	if (end_of_storage - finish >= 1)
	{
		if (pos != finish) {
			std::uninitialized_move(finish - 1, finish, finish);
			std::move_backward(pos, finish - 1, finish);
		}
		alloc::construct(pos, std::forward<T>(val));
		finish += 1;
	}
	else
	{
		size_type old_size = size();
		size_type new_size = old_size * 2;

		iterator new_start = Alloc::allocate(new_size);
		iterator new_finish = std::uninitialized_move(start, pos, new_start);
		alloc::construct(new_finish, std::forward<T>(val));
		new_finish += 1;
		new_finish = std::uninitialized_move(pos, finish, new_finish);

		alloc::destroy(start, finish);
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







