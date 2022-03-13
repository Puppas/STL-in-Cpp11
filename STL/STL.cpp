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
#include <atomic>
#include <set>
#include <map>
#include "rb_tree.h"
#include <any>
#include <execution>
#include "set.h"
#include "map.h"


using namespace std;

//using rb_tree_t = cx::rb_tree<cx::set_traits<int,std::less<int>, 
//		free_list_allocator<cx::rb_tree_node<int>>, true>>;

/*
void f(const rb_tree_t& tree)
{
	using iterator = rb_tree_t::iterator;
	std::queue<iterator> q;
	q.push(tree.root());
	while (!q.empty())
	{
		iterator iter = q.front();
		q.pop();
		if (iter.is_null()) {
			cout << "null" << ' ';
			continue;
		}

		cout << iter->value << ' ';
		q.push(iter->left);
		q.push(iter->right);
	}
	cout << endl;
}

int get_depth(rb_tree_t::iterator iter) {
	if (iter.is_null()) {
		return 0;
	}
	return max(get_depth(iter->left), get_depth(iter->right)) + 1;
}*/


template<typename T>
class A
{
public:
	using ty = T;
	A() = default;
	void f() {
		cout << "hello\n";
	}
};


template<typename T>
class B : public A<T>
{
public:
	B() = default;
	void g() {
		//using ty = typename A<T>::ty;
		typename A<T>::ty a = 10;
		cout << a;
		this->f();
	}
};
int main()
{
	cx::multimap<int, int> data;
	for (int i = 0; i < 10; ++i) {
		data.insert(std::make_pair(i, i));
	}
	for (int i = -10; i < 10; ++i) {
		data.insert(make_pair(i, i));
	}
	//data.erase(data.begin());
	for (auto iter = data.cbegin(); iter != data.cend(); ++iter)
	{
		cout << (*iter).second << '\n';
	}
	
	return 0;
}












