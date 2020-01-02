// STL.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include "cx_vector.h"
#include "cx_list.h"
#include "cx_deque.h"
#include <vector>
#include "cx_stack.h"

using namespace std;
/*
	在main中测试容器
*/

int main()
{
	cx_deque<int> data1({ 1, 2, 3 });
	cx_deque<int> data2({ 1, 2 });
	
	
	cout << (data1 == data2) << endl;

	data1.pop_back();

	cout << (data1 == data2) << endl;
	return 0;
}

