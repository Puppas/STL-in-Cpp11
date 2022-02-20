#pragma once
namespace cx {

struct input_iterator_tag{
	/*
	  Input iterators are iterators that can be used in sequential input 
	  operations, where each value pointed by the iterator is read only once 
	  and then the iterator is incremented.
	*/
};
struct output_iterator_tag {
	/*
	  Output iterators are iterators that can be used in sequential output 
	  operations, where each element pointed by the iterator is written a 
	  value only once and then the iterator is incremented.
	*/
};


struct forward_iterator_tag: public input_iterator_tag{};
struct bidirectional_iterator_tag: public forward_iterator_tag {};
struct random_access_iterator_tag: public bidirectional_iterator_tag {};


template<typename Iterator>
struct iterator_traits
{
	using iterator_category = typename Iterator::iterator_category;
	using value_type = typename Iterator::value_type;
	using pointer = typename Iterator::pointer;
	using reference = typename Iterator::reference;
	using difference_type = typename Iterator::difference_type;
};


template<typename T>
struct iterator_traits<T*>
{
	using iterator_category = random_access_iterator_tag;
	using value_type = T;
	using pointer = T*;
	using reference = T&;
	using difference_type = std::ptrdiff_t;
};


template<typename T>
struct iterator_traits<const T*>
{
	using iterator_category = random_access_iterator_tag;
	using value_type = T;
	using pointer = const T*;
	using reference = const T&;
	using difference_type = std::ptrdiff_t;
};


}