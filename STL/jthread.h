#pragma once

#include <thread>
#include <iostream>

class jthread
{
private:
	std::thread t;

public:
	jthread() noexcept = default;

	template<typename Callable, typename... Args>
	explicit jthread(Callable&& func, Args&&... args):
		t(std::forward<Callable>(func), std::forward<Args>(args)...)
	{}

	explicit jthread(std::thread t_) noexcept: t(std::move(t_))
	{}

	jthread(const jthread&) = delete;
	jthread(jthread&& other) noexcept: t(std::move(other.t))
	{}

	~jthread() noexcept
	{
		if (joinable()) {
			t.join();
		}
	}

	jthread& operator=(const jthread&) = delete;
	jthread& operator=(jthread&& other) noexcept
	{
		if (joinable())
			t.join();
		t = std::move(other.t);
		return *this;
	}

	jthread& operator=(std::thread t_) noexcept
	{
		if (joinable())
			t.join();
		t = std::move(t_);
		return *this;
	}

	bool joinable() const noexcept
	{
		return t.joinable();
	}

	void join()
	{
		t.join();
	}

	void swap(jthread& other) noexcept
	{
		t.swap(other.t);
	}

	std::thread::id get_id() const noexcept
	{
		return t.get_id();
	}

	void detach()
	{
		t.detach();
	}

	std::thread& as_thread() noexcept
	{
		return t;
	}

	const std::thread& as_thread() const noexcept
	{
		return t;
	}
};


