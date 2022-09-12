#pragma once
#include "malloc_allocator.h"
#include <iostream>


template<typename T>
class free_list_allocator
{
private:
	static constexpr std::size_t ALIGN = 8;
	static constexpr std::size_t MAX_BLOCK_SIZE = 128;
	static constexpr std::size_t FREE_LIST_NUM = MAX_BLOCK_SIZE / ALIGN;

	struct obj
	{
		obj *free_list_link;
	};

	static obj *free_list[FREE_LIST_NUM];
	//内存池起始位置
	static char *start_pool;
	//内存池结束位置
	static char *end_pool;
	static std::size_t heap_size;


	//将byte上调至8的倍数
	static std::size_t round_up(std::size_t byte) noexcept
	{
		return ((byte) + ALIGN - 1) & ~(ALIGN - 1);
	}

	//由所需内存大小决定free_list的索引
	static std::size_t free_list_index(std::size_t byte) noexcept
	{
		return  byte / (ALIGN + 1);
	}

	//为free_list加入新的块
	static T *refill(std::size_t block_size);

	//分配 num 个 block_size 的空间
	static char *chunk_alloc(std::size_t block_size, std::size_t& num);


public:
	typedef T value_type;

	free_list_allocator() {}
	template<typename U>
	free_list_allocator(const free_list_allocator<U>&) {}

	static T *allocate(std::size_t num);
	static void deallocate(T *p, std::size_t num) noexcept;
};


template<typename T>
typename free_list_allocator<T>::obj *
free_list_allocator<T>::free_list[FREE_LIST_NUM] = 0;


template<typename T>
char *free_list_allocator<T>::start_pool = nullptr;

template<typename T>
char *free_list_allocator<T>::end_pool = nullptr;

template<typename T>
std::size_t free_list_allocator<T>::heap_size = 0;


template<typename T>
T *free_list_allocator<T>::allocate(std::size_t num)
{	
	if (num * sizeof(T) > MAX_BLOCK_SIZE) {
		return malloc_allocator<T>::allocate(num);
	}

	obj *result;
	auto target_free_list = free_list + free_list_index(num * sizeof(T));
	result = *target_free_list;
	
	if (result == nullptr) {
		return refill(round_up(num * sizeof(T)));
	}

	//将free_list内的元素指向第2个内存块
	*target_free_list = result->free_list_link;
	return reinterpret_cast<T*>(result);
}


template<typename T>
void free_list_allocator<T>::deallocate(T *p, std::size_t num) noexcept
{
	/*
      p为需要回收的内存区域的首指针，num代表区域内元素的个数
	*/

	std::size_t size = num * sizeof(T);
	if (size > MAX_BLOCK_SIZE) {
		malloc_allocator<T>::deallocate(p, num);
		return;
	}
	
	//被回收的内存将被加入target_free_list指向的链表
	auto target_free_list = free_list + free_list_index(size);
	obj *target_block = reinterpret_cast<obj*>(p);
	target_block->free_list_link = *target_free_list;
	*target_free_list = target_block;
}



template<typename T>
T *free_list_allocator<T>::refill(std::size_t block_size)
{
	//默认分配16块内存
	std::size_t num = 16;
	//chunk指向新内存的起始地址
	char *chunk = chunk_alloc(block_size, num);   //num passed by reference

	if (num == 1) {
		return reinterpret_cast<T*>(chunk);
	}

	auto target_free_list = free_list + free_list_index(block_size);
	//第一块内存作为结果返回
	T *result = reinterpret_cast<T*>(chunk);
	--num;

	obj *cur_obj, *next_obj;
	chunk += block_size;
	cur_obj = reinterpret_cast<obj*>(chunk);
	cur_obj->free_list_link = nullptr;
	--num;
	*target_free_list = cur_obj;

	while (num > 0)
	{
		chunk += block_size;
		next_obj = reinterpret_cast<obj*>(chunk);
		next_obj->free_list_link = nullptr;
		--num;
		cur_obj->free_list_link = next_obj;
		cur_obj = next_obj;
	}

	return result;
}


template<typename T>
char *
free_list_allocator<T>::chunk_alloc(std::size_t block_size, std::size_t& num)
{
	char *result;
	/*
	total_byte: 总共需要的byte
	left_byte: 内存池剩余的byte
	*/
	std::size_t total_byte = block_size * num;
	std::size_t left_byte = end_pool - start_pool;

	if (left_byte > total_byte)
	{
		result = start_pool;
		start_pool += total_byte;
		return result;
	}
	else if (left_byte > block_size)
	{
		//num: 实际分配的块的个数
		num = left_byte / block_size;
		result = start_pool;
		start_pool += num * block_size;
		return result;
	}
	else
	{
		//内存池新大小
		std::size_t byte_to_get = 2 * total_byte + round_up(heap_size >> 4);

		if (left_byte > 0) {
			//将最后的空间分配出去
			obj *new_head = reinterpret_cast<obj*>(start_pool);
			auto target_free_list = free_list + free_list_index(left_byte);

			new_head->free_list_link = *target_free_list;
			*target_free_list = new_head;

			start_pool = end_pool = nullptr;
		}

		start_pool = static_cast<char*>(malloc(byte_to_get));
		if (start_pool == nullptr) {
			obj **target_free_list;
			obj *head;		//指向 free_list 的第一个内存块

			/*
			遍历块大小大于 block_size 的 free_list 以
			获取空闲的内存块加入到内存池中
			*/
			for (std::size_t i = block_size + ALIGN; i <= MAX_BLOCK_SIZE; i += ALIGN) {
				
				target_free_list = free_list + free_list_index(i);
				head = *target_free_list;

				if (head != nullptr) {
					/*
					如果有空闲块则加入到内存池中
					*/
					*target_free_list = head->free_list_link;
					start_pool = reinterpret_cast<char*>(head);
					end_pool = start_pool + i;
					return chunk_alloc(block_size, num);
				}

			}

			start_pool = reinterpret_cast<char*>(
					malloc_allocator<T>::allocate(byte_to_get / sizeof(T)));
		}

		end_pool = start_pool + byte_to_get;
		heap_size += byte_to_get;
		return chunk_alloc(block_size, num);
	}
}


template<typename T1, typename T2>
bool operator==(const free_list_allocator<T1>&, const free_list_allocator<T2>&)
{
	return true;
}

template<typename T1, typename T2>
bool operator!=(const free_list_allocator<T1>&, const free_list_allocator<T2>&)
{
	return false;
}
