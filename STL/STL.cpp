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
	cx_stack<int> data;

	data.push(1);
	data.push(2);
	data.push(3);

	cout << data.top() << endl;
	data.pop();

	cout << data.top() << endl;
	data.pop();
	cout << data.top() << endl;
	return 0;
}

