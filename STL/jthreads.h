#pragma once
#include <vector>
#include <thread>


class jthreads
{
private:
	std::vector<std::thread>& threads;

public:
	jthreads(std::vector<std::thread>& threads):
		threads(threads)
	{}

	~jthreads()
	{
		for (std::size_t i = 0; i < threads.size(); ++i) {
			if (threads[i].joinable())
				threads[i].join();
		}
	}
};
