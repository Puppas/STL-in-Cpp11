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


using namespace std;
/*
	在main中测试容器
*/

template<typename ForwardIterator, typename T>
struct accumulate_block
{
	void operator()(ForwardIterator first, ForwardIterator last, T& result)
	{
		result = std::accumulate(first, last, result);
	}
};


template<typename ForwardIterator, typename T>
T parallel_accumulate(ForwardIterator first, ForwardIterator last, T init)
{
	const std::size_t distance = std::distance(first, last);
	if (distance == 0)
		return init;

	const std::size_t min_num_per_thread = 32;
	const std::size_t max_thread_num =
		(distance + min_num_per_thread - 1) / min_num_per_thread;

	const std::size_t hardware_thread_num = std::thread::hardware_concurrency();

	const std::size_t thread_num = std::min(max_thread_num,
			hardware_thread_num != 0 ? hardware_thread_num : 2);
	
	const std::size_t num_per_thread = distance / thread_num;

	cx_vector<T> results(thread_num);
	cx_vector<std::thread> threads(thread_num - 1);

	ForwardIterator block_start = first;
	ForwardIterator block_end = block_start + num_per_thread;
	for (std::size_t i = 0; i < thread_num - 1; ++i)
	{
		threads[i] = std::thread(
			accumulate_block<ForwardIterator, T>(), 
			block_start, block_end, 
			std::ref(results[i]));

		block_start = block_end;
		block_end += num_per_thread;
	}

	accumulate_block<ForwardIterator, T>()(block_start, last, 
						results[thread_num - 1]);

	T result_sum = std::move(init) + std::move(results[thread_num - 1]);
	for (std::size_t i = 0; i < threads.size(); ++i)
	{
		threads[i].join();
		result_sum += std::move(results[i]);
	}

	return result_sum;
}


template<typename T>
list<T> parallel_sort(list<T> input)
{
	if (input.size() <= 1)
		return input;

	list<T> result;
	const T& pivot = *input.begin();
	result.splice(result.end(), input, input.begin());

	auto divide_point = std::partition(input.begin(), input.end(), 
				[&](const T& item) {
					return item < pivot;
				});
	list<T> lower_part;
	lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

	std::future<list<T>> sorted_lower_part(std::async(
		&parallel_sort<T>, std::move(lower_part)
	));
	
	input = parallel_sort(std::move(input));

	result.splice(result.end(), input);
	result.splice(result.begin(), sorted_lower_part.get());

	return result;
}




int main()
{	

	return 0;
}



