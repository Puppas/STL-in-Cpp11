#pragma once
#include <cstdlib>
#include <new>

template<typename T>
class malloc_allocator
{
	
public:
	typedef T value_type;

private:
	//oom: out of memory
	static T *oom_malloc(size_t n);
	static T *oom_realloc(T *p, size_t n);
	
	static void (*oom_handler)();


public:
	malloc_allocator() {}
	template<typename U>
	malloc_allocator(const malloc_allocator<U>&) {}

	static T *allocate(std::size_t num);
	static T *reallocate(T *p, std::size_t num);
	static void deallocate(T *p, std::size_t num);


	void (*set_oom_handler(void (*handler)()))();
};


template<typename T>
void (*malloc_allocator<T>::oom_handler)() = nullptr;


template<typename T>
T *malloc_allocator<T>::allocate(std::size_t num) 
{
	T *result = static_cast<T*>(malloc(num * sizeof(T)));
	if (result == nullptr) {
		result = oom_malloc(num * sizeof(T));
	}

	return result;
}


template<typename T>
T *malloc_allocator<T>::reallocate(T *p, std::size_t num) 
{
	T *result = static_cast<T*>(realloc(p, num * sizeof(T)));
	if (result == nullptr) {
		result = oom_realloc(p, num * sizeof(T));
	}

	return result;
}


template<typename T>
void malloc_allocator<T>::deallocate(T *p, std::size_t num) 
{
	free(p);
}

template<typename T>
void (*malloc_allocator<T>::set_oom_handler(void (*handler)()))()
{
	auto old_handler = oom_handler;
	oom_handler = handler;
	return old_handler;
}


template<typename T>
T *malloc_allocator<T>::oom_malloc(std::size_t n) 
{
	T *result = static_cast<T*>(malloc(n));

	while (result == nullptr)
	{
		if (oom_handler == nullptr)
			throw std::bad_alloc();
		oom_handler();

		result = static_cast<T*>(malloc(n));
	}

	return result;
}


template<typename T>
T *malloc_allocator<T>::oom_realloc(T *p, std::size_t n)
{
	T *result = static_cast<T*>(realloc(p, n));
	while (result == nullptr)
	{
		if (oom_handler == nullptr)
			throw std::bad_alloc();
		oom_handler();
		
		result = static_cast<T*>(realloc(p, n));
	}

	return result;
}


template<typename T1, typename T2>
bool operator==(const malloc_allocator<T1>&, const malloc_allocator<T2>&)
{
	return true;
}


template<typename T1, typename T2>
bool operator!=(const malloc_allocator<T1>&, const malloc_allocator<T2>&)
{
	return false;
}







