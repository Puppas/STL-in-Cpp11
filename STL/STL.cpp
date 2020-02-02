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
#include <cmath>
#include <future>
#include <atomic>
#include <thread>
#include <string>
#include <vector>


using namespace std;
/*
	在main中测试容器
*/

int main() {
	cx_vector<int> data{ 1, 2, 3 };

	for (int i = 4; i < 500; ++i)
		data.push_back(i);

	for (auto item : data)
		cout << item << endl;
	
	return 0;
}



