#pragma once
#include "iterator.h"
#include "free_list_allocator.h"
#include <cassert>


namespace cx {

using color_type = bool;
constexpr color_type RED = false;
constexpr color_type BLACK = true;


template<typename Traits>
class rb_tree_base_iterator;

template<typename Value>
struct rb_tree_iterator_traits;

template<typename Value>
using rb_tree_iterator = rb_tree_base_iterator<
	rb_tree_iterator_traits<Value>>;


template<typename Value>
struct rb_tree_node
{
	color_type color;
	rb_tree_iterator<Value> parent;
	rb_tree_iterator<Value> left;
	rb_tree_iterator<Value> right;
	Value value;
};


template<typename Value> 
struct rb_tree_iterator_traits
{
	using value_type = Value;
	using iterator_category = std::bidirectional_iterator_tag;
	using pointer = Value*;
	using reference = Value&;
	using difference_type = std::ptrdiff_t;
	using node_pointer = rb_tree_node<Value>*;        
};


template<typename Value>
struct rb_tree_const_iterator_traits
{
	using value_type = Value;
	using iterator_category = std::bidirectional_iterator_tag;
	using pointer = const Value*;
	using reference = const Value&;
	using difference_type = std::ptrdiff_t;
	using node_pointer = const rb_tree_node<Value>*;
};


template<typename Traits>
class rb_tree_base_iterator
{
public:
	template<typename TreeTraits>
	friend class rb_tree;

	template<typename IterTraits>
	friend class rb_tree_base_iterator;

	using value_type = typename Traits::value_type;
	using iterator_category = typename Traits::iterator_category;
	using pointer = typename Traits::pointer;
	using reference = typename Traits::reference;
	using difference_type = typename Traits::difference_type;

private:
	using iterator = rb_tree_base_iterator<Traits>;

protected:
	using node_pointer = typename Traits::node_pointer;
	node_pointer node_ptr;
	
public:
	rb_tree_base_iterator() noexcept : node_ptr(nullptr) {}
	rb_tree_base_iterator(node_pointer p) noexcept: node_ptr(p) {}
	
	rb_tree_base_iterator(const rb_tree_iterator<value_type>& iter) noexcept:
							node_ptr(iter.get_ptr()){}

	iterator& operator=(rb_tree_iterator<value_type> iter) noexcept {
		node_ptr = iter.get_ptr();
		return *this;
	}

	reference operator*() const noexcept { return node_ptr->value; }
	node_pointer operator->() const noexcept { return node_ptr; }

protected:
	bool is_null() const noexcept { return node_ptr == nullptr; }
	bool is_not_null() const noexcept { return !is_null(); }

	bool is_root() const noexcept {
		return is_not_null() && node_ptr->parent.is_header();
	}

	bool is_header() const noexcept{
		return is_not_null() && 
			(node_ptr->parent.is_null() || 
			   node_ptr->parent->parent == *this) && 
			node_ptr->color == RED;
	}

	bool is_left() const noexcept
	{
		return is_not_null() && !is_root() && !is_header() && 
			node_ptr->parent->left == *this;
	}

	bool is_right() const noexcept
	{
		return is_not_null() && !is_root() && !is_header() && 
			node_ptr->parent->right == *this;
	}

	node_pointer get_ptr() const noexcept { return node_ptr; }
	void clear() noexcept { node_ptr = nullptr; }

public:
	iterator& operator++() noexcept
	{
		iterator iter = *this;
		if (iter->right.is_not_null())
		{
			//如果有右结点存在，则向左查找最小结点
			iter = iter->right;
			while (iter->left.is_not_null())
				iter = iter->left;

			*this =  iter;
		}
		else
		{
			//当右节点不存在时，向上查找直到遇到左分支
			while (iter.is_right()) {
				iter = iter->parent;
			}

			*this = iter->parent;
		}

		return *this;
	}

	iterator& operator--() noexcept
	{
		//如果是header说明指向超尾，自减后应指向最大元素
		iterator iter = *this;
		if (iter.is_header())
		{
			*this = iter->right;
		}
		else if (iter->left.is_not_null()) {
			//如果左结点存在，则向右查找最大元素
			iter = iter->left;
			while (iter->right.is_not_null())
				iter = iter->right;

			*this = iter;
		}
		else {
			//左结点不存在时，向上查找直到遇到右分支
			while (iter.is_left()) {
				iter = iter->parent;
			}

			*this = iter->parent;
		}

		return *this;
	}

	iterator operator++(int) noexcept
	{
		iterator tmp = *this;
		this->operator++();
		return tmp;
	}

	iterator operator--(int) noexcept
	{
		iterator tmp = *this;
		this->operator--();
		return tmp;
	}
	
	template<typename Traits1, typename Traits2>
	friend bool operator==(rb_tree_base_iterator<Traits1> l,
		rb_tree_base_iterator<Traits2> r) noexcept;
	
	template<typename Traits1, typename Traits2>
	friend bool operator!=(rb_tree_base_iterator<Traits1> l,
		rb_tree_base_iterator<Traits2> r) noexcept;
};

template<typename Traits1, typename Traits2>
bool operator==(rb_tree_base_iterator<Traits1> l,
	rb_tree_base_iterator<Traits2> r) noexcept {
	return l.get_ptr() == r.get_ptr();
}

template<typename Traits1, typename Traits2>
bool operator!=(rb_tree_base_iterator<Traits1> l,
	rb_tree_base_iterator<Traits2> r) noexcept {
	return !(l == r);
}



template<typename Value>
using rb_tree_const_iterator = rb_tree_base_iterator<
	rb_tree_const_iterator_traits<Value>>;



template<typename Key, typename Compare, typename Alloc, 
		 bool multi>   // true if multiple equivalent keys are permitted
struct set_traits
{
	/*
	* make rb_tree behave like a set
	*/
	using key_type = Key;
	using value_type = Key;
	using key_compare = Compare;
	using value_compare = Compare;
	using allocator_type = Alloc;
	using iterator = rb_tree_const_iterator<value_type>;
	using const_iterator = rb_tree_const_iterator<value_type>;

	static constexpr bool MULTI = multi;

	class key_extractor {
	public:
		const key_type& operator()(const key_type& key) const noexcept{
			return key;
		}
	};
};


template<typename Key, typename T, typename Compare, 
		 typename Alloc, bool multi>
struct map_traits
{
	/*
	* make rb_tree behave like a map
	*/
	using key_type = Key;
	using value_type = std::pair<const Key, T>;
	using key_compare = Compare;
	using allocator_type = Alloc;
	using iterator = rb_tree_iterator<value_type>;
	using const_iterator = rb_tree_const_iterator<value_type>;

	static constexpr bool MULTI = multi;

	class key_extractor {
	public:
		const key_type& operator()(const value_type& value) const noexcept{
			return value.first;
		}
	};

	class value_compare{
	private:
		key_compare comp;
	public:
		value_compare() {}
		value_compare(const key_compare& comp): comp(comp) {}
		bool operator()(const value_type& l, const value_type& r) const{
			return comp(l.first, r.first);
		}
	};
};


template<typename Traits>
class rb_tree
{ 
public:
	using key_type = typename Traits::key_type;
	using value_type = typename Traits::value_type;
	using key_compare = typename Traits::key_compare;
	using value_compare = typename Traits::value_compare;
	using allocator_type = typename Traits::allocator_type;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = value_type*; //fix me: use allocator_traits instead
	using const_pointer = const value_type*;
	using iterator = typename Traits::iterator;
	using const_iterator = typename Traits::const_iterator;
	using difference_type = typename iterator_traits<iterator>::difference_type;
	using size_type = std::size_t;
	

protected:
	using key_extractor = typename Traits::key_extractor;
	using mut_iterator = rb_tree_iterator<value_type>;
	enum class side { left, right, parent };

protected:
	size_type node_count;
	mut_iterator header;
	key_compare comp;
	key_extractor extractor;

	mut_iterator create_node(color_type color);
	mut_iterator create_node(const value_type& val, color_type color) {
		return aux_create_node(val, color);
	}
	mut_iterator create_node(value_type&& val, color_type color) {
		return aux_create_node(std::move(val), color);
	}
	void destroy_node(mut_iterator iter) noexcept;
	mut_iterator root() const noexcept { return header->parent; }
	mut_iterator min() const noexcept { return header->left; }
	mut_iterator max() const noexcept { return header->right; }
	mut_iterator min(mut_iterator iter) const noexcept;
	mut_iterator max(mut_iterator iter) const noexcept;
	void rotate_left(mut_iterator x) noexcept;
	void rotate_right(mut_iterator x) noexcept;
	void transplant(mut_iterator x, mut_iterator y) noexcept;
	side get_side(mut_iterator iter) const noexcept;

private:
	void init();
	void destruct(mut_iterator iter) noexcept;
	mut_iterator aux_find(const key_type& key) const;

	template<typename T>
	mut_iterator aux_create_node(T&& val, color_type color);

	mut_iterator aux_lower_bound(const key_type& key) const;
	mut_iterator aux_upper_bound(const key_type& key) const;

	template<typename T>
	mut_iterator do_insert(mut_iterator y, T&& val, bool flag);
	
	template<typename T>
	std::pair<mut_iterator, bool> aux_insert_unique(T&& val);
	
	template<typename T>
	mut_iterator aux_insert_equal(T&& val);
	
	void insert_fixup(mut_iterator x) noexcept;
	void erase_fixup(mut_iterator x) noexcept;
	mut_iterator create_nil(mut_iterator x, side nil_side);
	

public:
	explicit rb_tree(const key_compare& comp = key_compare()): comp(comp), 
					 node_count(0) {
		init();
	}

	template<typename InputIterator>
	rb_tree(InputIterator first, InputIterator last,
			const key_compare& comp = key_compare());

	rb_tree(const rb_tree<Traits>& t);

	rb_tree(rb_tree<Traits>&& t) noexcept: 
		node_count(t.node_count), header(t.header), 
		comp(t.comp){
		t.node_count = 0;
		t.header.clear();
	}

	rb_tree(std::initializer_list<value_type> l,
		const key_compare& comp = key_compare()) : 
		comp(comp), node_count(0) {
		init();
		for (auto iter = l.begin(); iter != l.end(); ++iter) {
			if (Traits::MULTI) {
				insert_equal(*iter);
			}
			else {
				insert_unique(*iter);
			}
		}
	}

	~rb_tree(){
		//析构函数默认设置为noexcept
		if (header.is_not_null()) {
			destruct(root());
			destroy_node(header);
		}
	}

	key_compare key_comp() const noexcept { return comp; }
	iterator begin() noexcept { return header->left; }
	const_iterator begin() const noexcept { return header->left; }
	iterator end() noexcept { return header; }
	const_iterator end() const noexcept { return header; }
	const_iterator cbegin() const noexcept { return header->left; }
	const_iterator cend() const noexcept { return header; }
	bool empty() const noexcept { return node_count == 0; }
	size_type size() const noexcept { return node_count; }
	void clear() noexcept {
		destruct(root());
		header->left = header;
		header->right = header;
		header->parent.clear();
	}
	iterator find(const key_type& key) {
		return aux_find(key);
	}
	const_iterator find(const key_type& key) const {
		return aux_find(key);
	}

	size_type count(const key_type& key) const;

	iterator lower_bound(const key_type& key) {
		return aux_lower_bound(key);
	}

	const_iterator lower_bound(const key_type& key) const {
		return aux_lower_bound(key);
	}

	iterator upper_bound(const key_type& key) {
		return aux_upper_bound(key);
	}

	const_iterator upper_bound(const key_type& key) const{
		return aux_upper_bound(key);
	}

	std::pair<iterator, iterator>
	equal_range(const key_type& key) {
		return std::make_pair(lower_bound(key), upper_bound(key));
	}

	std::pair<const_iterator, const_iterator>
	equal_range(const key_type& key) const {
		return std::make_pair(lower_bound(key), upper_bound(key));
	}


	std::pair<iterator, bool> insert_unique(const value_type& val) {
		return aux_insert_unique(val);
	}
	std::pair<iterator, bool> insert_unique(value_type&& val) {
		return aux_insert_unique(std::move(val));
	}
	iterator insert_equal(const value_type& val) {
		return aux_insert_equal(val);
	}
	iterator insert_equal(value_type&& val) {
		return aux_insert_equal(std::move(val));
	}

	iterator erase(const_iterator iter) noexcept;

	void swap(rb_tree<Traits>& tree) noexcept{
		std::swap(node_count, tree.node_count);
		std::swap(header, tree.header);
	}

};



template<typename Traits>
typename rb_tree<Traits>::mut_iterator
rb_tree<Traits>::create_node(color_type color)
{
	mut_iterator iter(allocator_type::allocate(1));
	iter->color = color;
	iter->left.clear();
	iter->right.clear();
	iter->parent.clear();
	return iter;
}


template<typename Traits>
template<typename T>
typename rb_tree<Traits>::mut_iterator
rb_tree<Traits>::aux_create_node(T&& val, color_type color)
{
	mut_iterator iter(allocator_type::allocate(1));
	alloc::construct(&(iter->value), std::forward<T>(val));
	iter->color = color;
	iter->left.clear();
	iter->right.clear();
	iter->parent.clear();
	return iter;
}


template<typename Traits>
void rb_tree<Traits>::destroy_node(mut_iterator iter) noexcept
{
	alloc::destroy(&(iter->value));
	allocator_type::deallocate(iter.get_ptr(), 1);
}


template<typename Traits>
void cx::rb_tree<Traits>::destruct(mut_iterator iter) noexcept
{
	if (iter.is_null())
		return;

	destruct(iter->left);
	destruct(iter->right);
	destroy_node(iter);
	--node_count;
}


template<typename Traits>
void rb_tree<Traits>::init()
{
	header = create_node(RED);
	header->parent.clear();
	header->left = header;
	header->right = header;
}


template<typename Traits>
template<typename InputIterator>
rb_tree<Traits>::rb_tree(InputIterator first, InputIterator last,
	const key_compare& comp): node_count(0), comp(comp)
{
	init();
	for (auto iter = first; iter != last; ++iter) {
		if (Traits::MULTI) {
			insert_equal(*iter);
		}
		else {
			insert_unique(*iter);
		}
	}
}


template<typename Traits>
rb_tree<Traits>::rb_tree(const rb_tree<Traits>& t): 
	node_count(0), comp(t.comp)
{
	init();
	for (auto iter = t.cbegin(); iter != t.cend(); ++iter) {
		if (Traits::MULTI) {
			insert_equal(*iter);
		}
		else {
			insert_unique(*iter);
		}
	}
}


template<typename Traits>
typename rb_tree<Traits>::mut_iterator
rb_tree<Traits>::min(mut_iterator iter) const noexcept
{
	while (iter->left.is_not_null())
		iter = iter->left;
	return iter;
}


template<typename Traits>
typename rb_tree<Traits>::mut_iterator
rb_tree<Traits>::max(mut_iterator iter) const noexcept
{
	while (iter->right.is_not_null())
		iter = iter->right;
	return iter;
}


template<typename Traits>
void rb_tree<Traits>::rotate_left(mut_iterator x) noexcept
{
	mut_iterator y = x->right;
	x->right = y->left;
	if (y->left.is_not_null()) {
		y->left->parent = x;
	}
	
	transplant(x, y);
	x->parent = y;
	y->left = x;
}

template<typename Traits>
void rb_tree<Traits>::rotate_right(mut_iterator x) noexcept
{
	mut_iterator y = x->left;
	x->left = y->right;
	if (y->right.is_not_null()) {
		y->right->parent = x;
	}

	transplant(x, y);
	x->parent = y;
	y->right = x;
}


template<typename Traits>
typename rb_tree<Traits>::mut_iterator 
rb_tree<Traits>::aux_find(const key_type& key) const
{
	mut_iterator y = header;
	mut_iterator x = root();
	bool flag = true;
	while (x.is_not_null())
	{
		y = x;
		if (comp(key, extractor(x->value))) {
			x = x->left;
			flag = true;
		}
		else {
			x = x->right;
			flag = false;
		}
	}

	mut_iterator z = y;
	if (flag) {
		--z;
	}
	if (z.is_header() || comp(extractor(z->value), key)) {
		return header;
	}
	
	return z;
}


template<typename Traits>
typename rb_tree<Traits>::size_type 
rb_tree<Traits>::count(const key_type& key) const
{
	size_type num = 0;
	for (const_iterator iter = find(key); iter != header; --iter) {
		if (comp(iter->value, key)) {
			break;
		}
		++num;
	}

	return num;
}


template<typename Traits>
typename rb_tree<Traits>::mut_iterator
rb_tree<Traits>::aux_lower_bound(const key_type& key) const
{
	mut_iterator x = root();
	mut_iterator y = header;
	while (x.is_not_null())
	{
		if (!comp(x->value, key)) {
			y = x;
			x = x->left;
		}
		else { 
			x = x->right; 
		}
	}

	return y;
}



template<typename Traits>
typename rb_tree<Traits>::mut_iterator
rb_tree<Traits>::aux_upper_bound(const key_type& key) const
{
	mut_iterator x = root();
	mut_iterator y = header;
	while (x.is_not_null())
	{
		if (comp(key, x->value)) {
			y = x;
			x = x->left;
		}
		else {
			x = x->right;
		}
	}

	return y;
}

template<typename Traits>
template<typename T>
typename rb_tree<Traits>::mut_iterator
rb_tree<Traits>::aux_insert_equal(T&& val)
{
	static_assert(Traits::MULTI);
	mut_iterator y = header;
	mut_iterator x = root();
	bool flag = true;

	while (x.is_not_null())
	{
		y = x;
		if (comp(extractor(val), extractor(x->value))) {
			x = x->left;
			flag = true;
		}
		else {
			x = x->right;
			flag = false;
		}
	}

	return do_insert(y, std::forward<T>(val), flag);
}



template<typename Traits>
template<typename T>
std::pair<typename rb_tree<Traits>::mut_iterator, bool>
rb_tree<Traits>::aux_insert_unique(T&& val)
{
	static_assert(!Traits::MULTI);
	mut_iterator y = header;
	mut_iterator x = root();
	bool flag = true;
	while (x.is_not_null())
	{
		y = x;
		if (comp(extractor(val), extractor(x->value))) {
			x = x->left;
			flag = true;
		}
		else {
			x = x->right;
			flag = false;
		}
	}

	mut_iterator z = y;		//让z指向插入元素的前驱
	if (flag) {
		--z;
	}
	if (z.is_header() || comp(extractor(z->value), extractor(val))) {
		return std::make_pair(do_insert(y, 
			std::forward<T>(val), flag), true);
	}

	return std::make_pair(z, false);
}



template<typename Traits>
template<typename T>
typename rb_tree<Traits>::mut_iterator
rb_tree<Traits>::do_insert(mut_iterator y, T&& val, bool flag)
{
	mut_iterator x = create_node(std::forward<T>(val), RED);
	if (y.is_header()) {
		y->parent = x;
		x->parent = y;
		y->left = x;
		y->right = x;
	}
	else if (flag) {
		x->parent = y;
		y->left = x;
		if (y == min()) {
			header->left = x;
		}
	}
	else {
		x->parent = y;
		y->right = x;
		if (y == max()) {
			header->right = x;
		}
	}
	
	insert_fixup(x);
	++node_count;
	return x;
}


template<typename Traits>
void rb_tree<Traits>::insert_fixup(mut_iterator x) noexcept
{
	while (x->parent->color == RED && !(x.is_root()))
	{
		if (x->parent.is_left())
		{
			mut_iterator y = x->parent->parent->right; //y为x的叔结点
			if (y.is_not_null() && y->color == RED) {
				//当y的颜色为red
				x->parent->color = BLACK;
				x->parent->parent->color = RED;
				y->color = BLACK;
				x = x->parent->parent;
			}
			else {
				//当y不存在或为black
				if (x.is_right()) {
					x = x->parent;
					rotate_left(x);
				}
				x->parent->color = BLACK;
				x->parent->parent->color = RED;
				rotate_right(x->parent->parent);
			}
		}
		else
		{
			mut_iterator y = x->parent->parent->left;
			if (y.is_not_null() && y->color == RED) {
				//当y的颜色为red
				x->parent->color = BLACK;
				x->parent->parent->color = RED;
				y->color = BLACK;
				x = x->parent->parent;
			}
			else {
				//当y不存在或为black
				if (x.is_left()) {
					x = x->parent;
					rotate_right(x);
				}
				x->parent->color = BLACK;
				x->parent->parent->color = RED;
				rotate_left(x->parent->parent);
			}
		}
	}

	root()->color = BLACK;
}


template<typename Traits>
typename rb_tree<Traits>::side 
rb_tree<Traits>::get_side(mut_iterator iter) const noexcept
{
	if (iter.is_left()) {
		return side::left;
	}
	else if (iter.is_right()) {
		return side::right;
	}
	else {
		return side::parent;
	}
}


template<typename Traits>
typename rb_tree<Traits>::mut_iterator 
rb_tree<Traits>::create_nil(mut_iterator x, side nil_side)
{
	//create nil node in the right side
	mut_iterator iter = create_node(BLACK);
	switch (nil_side)
	{
	case side::left:
		x->left = iter;
		break;
	case side::right:
		x->right = iter;
		break;
	default:
		x->parent = iter;
		break;
	}
	iter->parent = x;
	return iter;
}


template<typename Traits>
void rb_tree<Traits>::transplant(mut_iterator x, mut_iterator y) noexcept
{
	if (x.is_left()) {
		x->parent->left = y;
	}
	else if (x.is_right()) {
		x->parent->right = y;
	}
	else {
		x->parent->parent = y;
	}

	if (y.is_not_null()) {
		y->parent = x->parent;
	}
}


template<typename Traits>
typename rb_tree<Traits>::iterator 
rb_tree<Traits>::erase(const_iterator iter) noexcept
{
	mut_iterator z(const_cast<typename mut_iterator::node_pointer>
		(iter.get_ptr()));
	mut_iterator x;
	mut_iterator ret = std::next(z);
	color_type color;
	bool flag = false;
	bool nil_flag = false; //whether create nil node
	side nil_side;

	if (z == min()) {
		header->left = std::next(z);
	}
	if (z == max()) {
		header->right = std::prev(z);
	}

	if (z->left.is_null()) {
		if (z->right.is_null()) {
			nil_side = get_side(z);
			x = z->parent;
			flag = true;
		}
		else {
			x = z->right;
		}
		color = z->color;
		transplant(z, z->right);
	}
	else if (z->right.is_null()) {
		x = z->left;
		color = z->color;
		transplant(z, z->left);
	}
	else {
		mut_iterator y = ret;
		if (y->right.is_not_null()) {
			x = y->right;
		}
		else if(y->parent != z) {
			nil_side = side::left;
			x = y->parent;
			flag = true;
		}
		else {
			nil_side = side::right;
			x = y;
			flag = true;
		}
		color = y->color;
		transplant(y, y->right);
		transplant(z, y);
		y->left = z->left;
		y->right = z->right;
		y->left->parent = y;
		y->color = z->color;
		if(y->right.is_not_null())
			y->right->parent = y;
	}

	if (color == BLACK) {
		if (flag) {
			x = create_nil(x, nil_side);
			nil_flag = true;
		}
		erase_fixup(x);
	}
	if (nil_flag) {
		transplant(x, mut_iterator());
		destroy_node(x);
	}

	destroy_node(z);
	--node_count;
	return ret;
}



template<typename Traits>
void rb_tree<Traits>::erase_fixup(mut_iterator x) noexcept
{
	while (!x.is_root() && x->color == BLACK)
	{
		if (x.is_left())
		{
			mut_iterator w = x->parent->right; //w为x的兄弟结点
			if (w->color == RED) {
				w->color = BLACK;
				x->parent->color = RED;
				rotate_left(x->parent);
				w = x->parent->right;
			}

			if ((w->left.is_null() && w->right.is_null()) ||
				(w->left.is_not_null() && w->left->color == BLACK && 
				 w->right.is_not_null() && w->right->color == BLACK)) {
				w->color = RED;
				x = x->parent;
			}
			else {
				if (w->right.is_null() || w->right->color == BLACK) {
					w->left->color = BLACK;
					w->color = RED;
					rotate_right(w);
					w = x->parent->right;
				}

				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->right->color = BLACK;
				rotate_left(x->parent);
				x = root();
			}
		}
		else
		{
			mut_iterator w = x->parent->left; //w为x的兄弟结点
			if (w->color == RED) {
				w->color = BLACK;
				x->parent->color = RED;
				rotate_right(x->parent);
				w = x->parent->left;
			}

			if ((w->left.is_null() && w->right.is_null()) ||
				(w->left.is_not_null() && w->left->color == BLACK &&
				 w->right.is_not_null() && w->right->color == BLACK)) {
				w->color = RED;
				x = x->parent;
			}
			else {
				if (w->left.is_null() || w->left->color == BLACK) {
					w->right->color = BLACK;
					w->color = RED;
					rotate_left(w);
					w = x->parent->left;
				}

				w->color = x->parent->color;
				x->parent->color = BLACK;
				w->left->color = BLACK;
				rotate_right(x->parent);
				x = root();
			}
		}
	}
	x->color = BLACK;
}




template<typename Traits>
inline void swap(rb_tree<Traits>& l, rb_tree<Traits>& r) noexcept
{
	l.swap(r);
}
}


