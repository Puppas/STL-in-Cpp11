// STL.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "cx_vector.h"
#include "cx_list.h"
#include "cx_deque.h"
#include "cx_stack.h"
#include "cx_queue.h"
#include "thread_queue.h"
#include "thread_stack.h"
#include <cmath>
#include <future>
#include <atomic>
#include <thread>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include "thread_pool.h"
#include <cstdlib>
#include <ctime>
#include "cx_shared_ptr.h"
#include <cstdio>
#include <cstring>
#include <chrono>
#include "jthread.h"


template<typename ForwardIterator, typename T>
struct accumulate_block
{
	T operator()(ForwardIterator first, ForwardIterator last)
	{
		return std::accumulate(first, last, 0);
	}
};


template<typename ForwardIterator, typename T>
T parallel_accumulate(ForwardIterator first, ForwardIterator last, T init)
{
	const std::size_t distance = std::distance(first, last);
	if (distance == 0)
		return init;

	const std::size_t block_size = 25;
	const std::size_t block_count = (distance + block_size - 1) / block_size;

	std::vector<std::future<T>> futures(block_count - 1);
	thread_pool pool;

	ForwardIterator block_start = first;
	for (std::size_t i = 0; i < block_count - 1; ++i)
	{
		ForwardIterator block_end = block_start + block_size;

		futures[i] = pool.submit([=]() {
			return accumulate_block<ForwardIterator, T>()(
				block_start, block_end);
		});

		block_start = block_end;
	}

	T last_result = accumulate_block<ForwardIterator, T>()(block_start, last);

	T result_sum = std::move(init) + std::move(last_result);
	for (std::size_t i = 0; i < futures.size(); ++i)
	{
		result_sum += futures[i].get();
	}

	return result_sum;
}


template<typename T>
struct sorter
{
	thread_pool pool;

	std::list<T> sort(std::list<T> data)
	{
		if (data.empty())
			return data;

		std::list<T> result;
		const T& pivot = *data.begin();
		result.splice(result.end(), data, data.begin());

		auto divide_point = std::partition(data.begin(), data.end(),
			[&](const T& item) {
			return item < pivot;
		});

		std::list<T> lower_part;
		lower_part.splice(lower_part.end(), data, data.begin(), divide_point);
		std::future<std::list<T>> lower_future =
			pool.submit(std::move(std::bind(
				&sorter::sort, this, std::move(lower_part))));

		data = sort(std::move(data));

		while (lower_future.wait_for(std::chrono::seconds(0)) !=
			std::future_status::ready) {
			pool.run_task();
		}

		result.splice(result.begin(), lower_future.get());
		result.splice(result.end(), data);

		return result;
	}
};

template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input)
{
	if (input.empty())
	{
		return input;
	}
	sorter<T> s;

	return s.sort(input);
}


template<typename ForwardIterator, typename Func>
void parallel_for_each(ForwardIterator first, ForwardIterator last,
	Func f)
{
	const std::size_t min_count_per_thread = 32;
	const std::size_t count = std::distance(first, last);

	if (count < 2 * min_count_per_thread)
	{
		std::for_each(first, last, f);
		return;
	}

	ForwardIterator mid = first;
	std::advance(mid, count / 2);

	std::future<void> future = std::async(
		&parallel_for_each<ForwardIterator, Func>, first, mid, f);

	parallel_for_each(mid, last, f);
	future.get();
}


using namespace std;





int main()
{
	cx_deque<int> data({ 1,2,0,4 });
	for (int i = 0; i < 1000; ++i) {
		data.push_back(i);
		data.push_front(i - 1000);
	}
	for (auto iter = data.cbegin(); iter != data.cend(); ++iter)
		cout << *iter << endl;
 	return 0;
}

















