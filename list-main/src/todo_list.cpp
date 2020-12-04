// Copyright (c) 2020 udv. All rights reserved.

#include <utility>

#include "udv/todo_list.hpp"

#define GROW(x) x * 2

namespace udv {

	TodoList &TodoList::operator=(const TodoList &other) {
		if (this == &other) {
			return *this;
		}

		this->mItems = new item_type[other.mCapacity];
		this->mCapacity = other.mCapacity;

		std::copy(other.mItems, other.mItems + other.mSize, this->mItems);
		this->mSize = other.mSize;

		return *this;
	}

	TodoList &TodoList::operator=(TodoList &&other) noexcept {
		if (this == &other) {
			return *this;
		}

		this->mItems = other.mItems;
		other.mItems = nullptr;

		this->mCapacity = other.mCapacity;
		other.mCapacity = 0;

		this->mSize = other.mSize;
		other.mSize = 0;

		return *this;
	}

	void TodoList::addItem(TodoItem item) {
		if (mSize >= mCapacity) {
			grow();
		}

		*end() = std::move(item);
		++mSize;
	}

	template<typename... Args>
	void TodoList::addItem(Args &&... args) {
		if (mSize >= mCapacity) {
			grow();
		}

		*end() = TodoItem{std::forward<Args>(args)...};
		++mSize;
	}

	void TodoList::grow() {
		size_type new_capacity = mCapacity != 0 ? GROW(mCapacity) : 1;
		mItems = new item_type[new_capacity];
		mCapacity = new_capacity;
	}
}
