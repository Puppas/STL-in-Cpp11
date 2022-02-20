#pragma once
#include <functional>

template<typename T>
class Deleter
{
public:
	void operator()(T *p) {
		std::cout << "asd" << std::endl;
		delete p;
	}
};


template<typename T>
class cx_shared_ptr
{
private:
	T *ptr;
	std::size_t *count;
	std::function<void(T*)> deleter;

public:
	cx_shared_ptr() noexcept: ptr(nullptr), count(nullptr),
		deleter(Deleter<T>()) {}

	cx_shared_ptr(T *ptr, const std::function<void(T*)>& deleter
		= Deleter<T>()): ptr(ptr), count(new std::size_t(ptr != nullptr)),
		deleter(deleter){}

	cx_shared_ptr(const cx_shared_ptr<T>& rhs): ptr(rhs.ptr), 
		count(rhs.count), deleter(rhs.deleter){
		if(count)
			++*count;
	}

	cx_shared_ptr& operator=(const cx_shared_ptr& rhs) {
		cx_shared_ptr temp(rhs);
		temp.swap(*this);
		return *this;
	}

	cx_shared_ptr(cx_shared_ptr<T>&& rhs): ptr(rhs.ptr),
		count(rhs.count), deleter(std::move(rhs.deleter))
	{
		rhs.count = nullptr;
		rhs.ptr = nullptr;
	}

	cx_shared_ptr& operator=(cx_shared_ptr&& rhs) {
		cx_shared_ptr<T> temp(std::move(rhs));
		temp.swap(*this);
		return *this;
	}


	~cx_shared_ptr() {
		reset();
	}

	void swap(cx_shared_ptr<T>& rhs) noexcept {
		using std::swap;
		swap(ptr, rhs.ptr);
		swap(count, rhs.count);
		swap(deleter, rhs.deleter);
	}

	std::size_t use_count() const noexcept {
		return count ? *count : 0;
	}

	void reset() {
		if (count) {
			--*count;
			if (*count == 0) {
				deleter(ptr);
				delete count;
			}
		}

		ptr = nullptr;
		count = nullptr;
	}

	void reset(T *ptr, const std::function<void(T*)>& deleter = Deleter<T>()){
		reset();
		this->ptr = ptr;
		this->count = new std::size_t(ptr != nullptr);
		this->deleter = deleter;
	}

	T *get() const noexcept {
		return ptr;
	}

	T& operator*() const noexcept {
		return *ptr;
	}

	T *operator->() const noexcept {
		return ptr;
	}

	bool unique() const noexcept {
		return count ? *count == 1 : true;
	}

	operator bool() const noexcept {
		return ptr != nullptr;
	}
};