// STL.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "cx_vector.h"
#include "cx_list.h"
#include "cx_deque.h"
#include <vector>

using namespace std;
/*
	在main中测试容器
*/

int main()
{
	cx_deque<double> container(40, 1);

	container.push_front(0);
	container.push_back(5);

	for (auto item : container)
		cout << item << endl;
	return 0;
}

