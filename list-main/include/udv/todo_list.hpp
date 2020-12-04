// Copyright (c) 2020 udv. All rights reserved.

#ifndef TODO_LIST_TODO_LIST
#define TODO_LIST_TODO_LIST

#include "todo_item.hpp"

namespace udv {
	class TodoList {
		using size_type = std::size_t;
		using item_type = TodoItem;
	public:
		TodoList() : mItems{nullptr}, mSize{0}, mCapacity{0} {}
		explicit TodoList(size_type reserve_size)
				: mItems{new item_type[reserve_size]}, mSize{0}, mCapacity{reserve_size} {}

		TodoList(const TodoList &other) : TodoList() { *this = other; }
		TodoList(TodoList &&other) noexcept: TodoList() { *this = std::move(other); }

		TodoList &operator=(const TodoList &other);
		TodoList &operator=(TodoList &&other) noexcept;
	private:
		item_type* begin() { return mItems; }
		item_type* end() { return mItems + mSize; }
	private:
		void grow();
	public:
		void addItem(item_type item);

		template<typename... Args>
		void addItem(Args &&... args);
	public:
		struct NotifierGuard {
			// TODO: implement this
		};
	private:
		item_type *mItems;
		size_type mSize;
		size_type mCapacity;
	};
}

#endif //TODO_LIST_TODO_LIST
