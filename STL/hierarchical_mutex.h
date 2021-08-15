#pragma once

#include <mutex>
#include <limits>

class hierarchical_mutex
{
private:
	std::mutex internal_mutex;
	const std::size_t hierarchy_value;
	std::size_t previous_hierarchy_value;

	static thread_local std::size_t this_thread_hierarchy_value;
	static const std::size_t MAX_HIERARCHY_VALUE =
		std::numeric_limits<std::size_t>::max();

	void check_hierarchy_violation() const
	{
		if (hierarchy_value >= this_thread_hierarchy_value)
			throw std::logic_error("mutex hierarchy violated");
	}

	void update_hierarchy_value() noexcept
	{
		previous_hierarchy_value = this_thread_hierarchy_value;
		this_thread_hierarchy_value = hierarchy_value;
	}

public:
	explicit hierarchical_mutex(std::size_t value) :
		hierarchy_value(value), previous_hierarchy_value(MAX_HIERARCHY_VALUE)
	{}

	void lock()
	{
		check_hierarchy_violation();
		internal_mutex.lock();
		update_hierarchy_value();
	}

	void unlock()
	{
		if (this_thread_hierarchy_value != hierarchy_value)
			throw std::logic_error("mutex hierarchy violated");
		this_thread_hierarchy_value = previous_hierarchy_value;
		internal_mutex.unlock();
	}

	bool try_lock()
	{
		check_hierarchy_violation();
		if (!internal_mutex.try_lock())
			return false;
		update_hierarchy_value();
		return true;
	}
};

thread_local std::size_t
hierarchical_mutex::this_thread_hierarchy_value = 
hierarchical_mutex::MAX_HIERARCHY_VALUE;