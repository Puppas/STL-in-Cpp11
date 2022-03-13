#pragma once
#include "free_list_allocator.h"
#include "rb_tree.h"
namespace cx
{
template<typename Key, typename T, typename Compare = std::less<Key>,
	     typename Alloc = free_list_allocator<
			rb_tree_node<std::pair<const Key, T>>>>
class map
{
protected:
	using rb_tree_t = rb_tree<map_traits<Key, T, Compare, Alloc, false>>;
	rb_tree_t tree;

public:
	using key_type = typename rb_tree_t::key_type;
	using mapped_type = T;
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
	explicit map(const key_compare& comp = key_compare()) : tree(comp) {}

	template<typename InputIterator>
	map(InputIterator first, InputIterator last,
		const key_compare& comp = key_compare()) : tree(first, last, comp) {}

	map(const map& m) : tree(m.tree) {}

	map(map&& m) noexcept : tree(std::move(m.tree)) {}

	map(std::initializer_list<value_type> l,
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

	void swap(map& m) noexcept {
		tree.swap(m.tree);
	}
};


template<typename Key, typename T, typename Compare = std::less<T>,
		 typename Alloc = free_list_allocator<
			rb_tree_node<std::pair<const Key, T>>>>
class multimap
{
protected:
	using rb_tree_t = rb_tree<map_traits<Key, T, Compare, Alloc, true>>;
	rb_tree_t tree;

public:
	using key_type = typename rb_tree_t::key_type;
	using mapped_type = T;
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
	explicit multimap(const key_compare& comp = key_compare()) : tree(comp) {}

	template<typename InputIterator>
	multimap(InputIterator first, InputIterator last,
		const key_compare& comp = key_compare()) : tree(first, last, comp) {}

	multimap(const multimap& m) : tree(m.tree) {}

	multimap(multimap&& m) noexcept : tree(std::move(m.tree)) {}

	multimap(std::initializer_list<value_type> l,
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

	void swap(multimap& m) noexcept {
		tree.swap(m.tree);
	}
};
}