#pragma once
#include "free_list_allocator.h"
#include "rb_tree.h"

namespace cx
{
template<typename T, typename Compare = std::less<T>, 
		 typename Alloc = free_list_allocator<rb_tree_node<T>>>
class set
{
protected:
	using rb_tree_t = rb_tree<set_traits<T, Compare, Alloc, false>>;
	rb_tree_t tree;

public:
	using key_type = typename rb_tree_t::key_type;
	using value_type = typename rb_tree_t::value_type;
	using key_compare = typename rb_tree_t::key_compare;
	using value_compare = typename rb_tree_t::value_compare;
	using allocator_type = typename rb_tree_t::allocator_type;
	using reference = typename rb_tree_t::reference;
	using const_reference = typename rb_tree_t::const_reference;
	using pointer = typename rb_tree_t::pointer; 
	using const_pointer = typename rb_tree_t::const_pointer;
	using iterator = typename rb_tree_t::iterator;
	using const_iterator = typename rb_tree_t::const_iterator;
	using difference_type = typename rb_tree_t::difference_type;
	using size_type = typename rb_tree_t::size_type;

public:
	explicit set(const key_compare& comp = key_compare()): tree(comp) {}

	template<typename InputIterator>
	set(InputIterator first, InputIterator last,
		const key_compare& comp = key_compare()): tree(first, last, comp){}

	set(const set& s): tree(s.tree) {}

	set(set&& s) noexcept : tree(std::move(s.tree)) {}

	set(std::initializer_list<value_type> l,
		const key_compare& comp = key_compare()) :
		tree(l, comp) {}

	key_compare key_comp() const noexcept { return tree.comp; }
	iterator begin() noexcept { return tree.begin(); }
	const_iterator begin() const noexcept { return tree.begin(); }
	iterator end() noexcept { return tree.end(); }
	const_iterator end() const noexcept { return tree.end(); }
	const_iterator cbegin() const noexcept { return tree.cbegin(); }
	const_iterator cend() const noexcept { return tree.cend(); }
	bool empty() const noexcept { return tree.empty(); }
	size_type size() const noexcept { return tree.size(); }
	void clear() noexcept { tree.clear(); }
	iterator find(const key_type& key) { return tree.find(key); }
	const_iterator find(const key_type& key) const {
		return tree.find(key);
	}

	size_type count(const key_type& key) const {
		return tree.count(key);
	}

	iterator lower_bound(const key_type& key) {
		return tree.lower_bound(key);
	}

	const_iterator lower_bound(const key_type& key) const {
		return tree.lower_bound(key);
	}

	iterator upper_bound(const key_type& key) {
		return tree.upper_bound(key);
	}

	const_iterator upper_bound(const key_type& key) const {
		return tree.upper_bound(key);
	}

	std::pair<iterator, iterator>
		equal_range(const key_type& key) {
		return tree.equal_range(key);
	}

	std::pair<const_iterator, const_iterator>
		equal_range(const key_type& key) const {
		return tree.equal_range(key);
	}


	std::pair<iterator, bool> insert(const value_type& val) {
		return tree.insert_unique(val);
	}
	std::pair<iterator, bool> insert(value_type&& val) {
		return tree.insert_unique(std::move(val));
	}

	iterator erase(const_iterator iter) noexcept {
		return tree.erase(iter);
	}

	void swap(set& s) noexcept {
		tree.swap(s.tree);
	}
};


template<typename T, typename Compare = std::less<T>,
		 typename Alloc = free_list_allocator<rb_tree_node<T>>>
class multiset
{
protected:
	using rb_tree_t = rb_tree<set_traits<T, Compare, Alloc, true>>;
	rb_tree_t tree;

public:
	using key_type = typename rb_tree_t::key_type;
	using value_type = typename rb_tree_t::value_type;
	using key_compare = typename rb_tree_t::key_compare;
	using value_compare = typename rb_tree_t::value_compare;
	using allocator_type = typename rb_tree_t::allocator_type;
	using reference = typename rb_tree_t::reference;
	using const_reference = typename rb_tree_t::const_reference;
	using pointer = typename rb_tree_t::pointer;
	using const_pointer = typename rb_tree_t::const_pointer;
	using iterator = typename rb_tree_t::iterator;
	using const_iterator = typename rb_tree_t::const_iterator;
	using difference_type = typename rb_tree_t::difference_type;
	using size_type = typename rb_tree_t::size_type;

public:
	explicit multiset(const key_compare& comp = key_compare()) : tree(comp) {}

	template<typename InputIterator>
	multiset(InputIterator first, InputIterator last,
		const key_compare& comp = key_compare()) : tree(first, last, comp) {}

	multiset(const multiset& s) : tree(s.tree) {}

	multiset(multiset&& s) noexcept : tree(std::move(s.tree)) {}

	multiset(std::initializer_list<value_type> l,
		const key_compare& comp = key_compare()) :
		tree(l, comp) {}

	key_compare key_comp() const noexcept { return tree.comp; }
	iterator begin() noexcept { return tree.begin(); }
	const_iterator begin() const noexcept { return tree.begin(); }
	iterator end() noexcept { return tree.end(); }
	const_iterator end() const noexcept { return tree.end(); }
	const_iterator cbegin() const noexcept { return tree.cbegin(); }
	const_iterator cend() const noexcept { return tree.cend(); }
	bool empty() const noexcept { return tree.empty(); }
	size_type size() const noexcept { return tree.size(); }
	void clear() noexcept { tree.clear(); }
	iterator find(const key_type& key) { return tree.find(key); }
	const_iterator find(const key_type& key) const {
		return tree.find(key);
	}

	size_type count(const key_type& key) const {
		return tree.count(key);
	}

	iterator lower_bound(const key_type& key) {
		return tree.lower_bound(key);
	}

	const_iterator lower_bound(const key_type& key) const {
		return tree.lower_bound(key);
	}

	iterator upper_bound(const key_type& key) {
		return tree.upper_bound(key);
	}

	const_iterator upper_bound(const key_type& key) const {
		return tree.upper_bound(key);
	}

	std::pair<iterator, iterator>
		equal_range(const key_type& key) {
		return tree.equal_range(key);
	}

	std::pair<const_iterator, const_iterator>
		equal_range(const key_type& key) const {
		return tree.equal_range(key);
	}


	iterator insert(const value_type& val) {
		return tree.insert_equal(val);
	}
	iterator insert(value_type&& val) {
		return tree.insert_equal(std::move(val));
	}

	iterator erase(const_iterator iter) noexcept {
		return tree.erase(iter);
	}

	void swap(multiset& s) noexcept {
		tree.swap(s.tree);
	}
};
}