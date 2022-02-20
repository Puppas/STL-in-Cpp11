#pragma once
#include <type_traits>

namespace alloc {

	template<typename T1, typename T2>
	void construct(T1* p, const T2& value)
	{
		new (p) T1(value);
	}

	template<typename T1, typename T2>
	void construct(T1 *p, T2&& value)
	{
		new (p) T1(std::forward<T2>(value));
	}

	template<typename T>
	void destroy(T *p) noexcept
	{
		p->~T();
	}


	template<typename ForwardIterator>
	void aux_destroy(ForwardIterator begin, ForwardIterator end, std::true_type)
	{
	}


	template<typename ForwardIterator>
	void aux_destroy(ForwardIterator begin, ForwardIterator end, 
					 std::false_type) noexcept
	{
		for (; begin != end; ++begin) {
			destroy(&*begin);
		}
	}


	template<typename ForwardIterator>
	void destroy(ForwardIterator begin, ForwardIterator end) noexcept
	{
		aux_destroy(begin, end, std::is_trivially_destructible<decltype(*begin)>());
	}
}


