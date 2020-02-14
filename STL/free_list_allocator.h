#pragma once
#include "malloc_allocator.h"
#include <iostream>


template<typename T>
class free_list_allocator
{
private:
	static const std::size_t ALIGN = 8;
	static const std::size_t MAX_BLOCK_SIZE = 128;
	static const std::size_t FREE_LIST_NUM = MAX_BLOCK_SIZE / ALIGN;

	struct obj
	{
		obj *free_list_link;
	};

	static obj *free_list[FREE_LIST_NUM];
	//�ڴ����ʼλ��
	static char *start_pool;
	//�ڴ�ؽ���λ��
	static char *end_pool;
	static std::size_t heap_size;


	//��byte�ϵ���8�ı���
	static std::size_t round_up(std::size_t byte)
	{
		return ((byte) + ALIGN - 1) & ~(ALIGN - 1);
	}

	//�������ڴ��С����free_list������
	static std::size_t free_list_index(std::size_t byte)
	{
		return  byte / (ALIGN + 1);
	}

	//Ϊfree_list�����µĿ�
	static T *refill(std::size_t block_size);

	//���� num �� block_size �Ŀռ�
	static char *chunk_alloc(std::size_t block_size, std::size_t& num);


public:
	typedef T value_type;

	free_list_allocator() {}
	template<typename U>
	free_list_allocator(const free_list_allocator<U>&) {}

	static T *allocate(std::size_t num);
	static void deallocate(T *p, std::size_t num);
};


template<typename T>
typename free_list_allocator<T>::obj *
free_list_allocator<T>::free_list[FREE_LIST_NUM] = {
	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
};


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

	//��free_list�ڵ�Ԫ��ָ���2���ڴ��
	*target_free_list = result->free_list_link;
	return reinterpret_cast<T*>(result);
}


template<typename T>
void free_list_allocator<T>::deallocate(T *p, std::size_t num)
{
	/*
	pΪ��Ҫ���յ��ڴ��������ָ�룬num����������Ԫ�صĸ���
	*/

	std::size_t size = num * sizeof(T);
	if (size > MAX_BLOCK_SIZE) {
		malloc_allocator<T>::deallocate(p, num);
		return;
	}
	
	//�����յ��ڴ潫������target_free_listָ�������
	auto target_free_list = free_list + free_list_index(size);
	obj *target_block = reinterpret_cast<obj*>(p);
	target_block->free_list_link = *target_free_list;
	*target_free_list = target_block;
}



template<typename T>
T *free_list_allocator<T>::refill(std::size_t block_size)
{
	//Ĭ�Ϸ���16���ڴ�
	std::size_t num = 16;
	//chunkָ�����ڴ����ʼ��ַ
	char *chunk = chunk_alloc(block_size, num);   //num passed by reference

	if (num == 1) {
		return reinterpret_cast<T*>(chunk);
	}

	auto target_free_list = free_list + free_list_index(block_size);
	//��һ���ڴ���Ϊ�������
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
	total_byte: �ܹ���Ҫ��byte
	left_byte: �ڴ��ʣ���byte
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
		//num: ʵ�ʷ���Ŀ�ĸ���
		num = left_byte / block_size;
		result = start_pool;
		start_pool += num * block_size;
		return result;
	}
	else
	{
		//�ڴ���´�С
		std::size_t byte_to_get = 2 * total_byte + round_up(heap_size >> 4);

		if (left_byte > 0) {
			//�����Ŀռ�����ȥ
			obj *new_head = reinterpret_cast<obj*>(start_pool);
			auto target_free_list = free_list + free_list_index(left_byte);

			new_head->free_list_link = *target_free_list;
			*target_free_list = new_head;

			start_pool = end_pool = nullptr;
		}

		start_pool = static_cast<char*>(malloc(byte_to_get));
		if (start_pool == nullptr) {
			obj **target_free_list;
			obj *head;		//ָ�� free_list �ĵ�һ���ڴ��

			/*
			�������С���� block_size �� free_list ��
			��ȡ���е��ڴ����뵽�ڴ����
			*/
			for (std::size_t i = block_size + ALIGN; i <= MAX_BLOCK_SIZE; i += ALIGN) {
				
				target_free_list = free_list + free_list_index(i);
				head = *target_free_list;

				if (head != nullptr) {
					/*
					����п��п�����뵽�ڴ����
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
