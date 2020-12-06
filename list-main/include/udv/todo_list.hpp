// Copyright (c) 2020 udv. All rights reserved.

#ifndef TODO_LIST_TODO_LIST
#define TODO_LIST_TODO_LIST

#include <functional>
#include <utility>
#include <mutex>
#include <thread>
#include <vector>
#include <algorithm>

#include "todo_item.hpp"

#define DEFAULT_EXPORT_FILENAME "settings.todo"

namespace udv {

	class TodoList {
		using size_type = std::size_t;
		using item_type = TodoItem;
		using callback_type = std::function<void(item_type)>;
	public:
		struct NotifierGuard {
			TodoList &list;

			NotifierGuard(TodoList &list_, callback_type callback)
					: list{list_} {
				list.setCallbackNotifier(callback);
			}
			
			~NotifierGuard() {
				list.detachCallback();
			}
		};

	public:
		explicit TodoList(const std::string &filename = DEFAULT_EXPORT_FILENAME, bool exportOnDeath = true)
				: mItems{}, mWorking{false}, mExport{exportOnDeath} {
			importSettings(filename);
			initThread();
		}

		~TodoList();

		TodoList(const TodoList &other) : TodoList() { *this = other; }
		TodoList(TodoList &&other) noexcept: TodoList() { *this = std::move(other); }

		TodoList &operator=(const TodoList &other);
		TodoList &operator=(TodoList &&other) noexcept;

		NotifierGuard &guard(const callback_type &callback) {
			return *(new NotifierGuard{*this, callback});
		}
	private:
		void initThread() {
			mWorking = true;
			mRunningThread = std::thread{routine, std::ref(*this)};
		}
		static void routine(TodoList &list);
		void tryTrigger();
	private:
		void exportSettings(const std::string &filename = DEFAULT_EXPORT_FILENAME);
		void importSettings(const std::string &filename = DEFAULT_EXPORT_FILENAME);
	public:
		template<typename... Args>
		inline void addItem(Args &&... args) {
			std::lock_guard guard{mMutex};

			mItems.emplace_back(std::forward<Args>(args)...);

			std::sort(mItems.begin(), mItems.end(), [](const item_type &o1, const item_type &o2) {
				return o1.triggerTime > o2.triggerTime;
			});
		}

		[[nodiscard]] size_type size() const { return mItems.size(); }
		[[nodiscard]] bool empty() const { return mItems.empty(); }

	private:
		void setCallbackNotifier(const TodoList::callback_type& callback);
		void detachCallback();
	private:
		std::vector<item_type> mItems;
		bool mExport;

		callback_type mCallback;

		bool mWorking;
		std::mutex mMutex;
		std::thread mRunningThread;
	};
}

#endif //TODO_LIST_TODO_LIST
