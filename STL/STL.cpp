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


using namespace std;

using rb_tree_t = cx::rb_tree<cx::set_traits<int,std::less<int>, 
		free_list_allocator<cx::rb_tree_node<int>>, true>>;

/*
void f(const rb_tree_t& tree)
{
	using iterator = rb_tree_t::iterator;
	std::queue<iterator> q;
	//q.push(tree.root());
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
*/

class A
{
private:
	void f() const{
		std::cout << 'A';
	}

public:
	void g(const A& a) {
		a.f();
	}
};

int main()
{
	rb_tree_t tree;
	tree.insert_equal(12);
	rb_tree_t::iterator iter;
	
	A a1, a2;
	a1.g(a2);
	return 0;
}












