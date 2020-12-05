// Copyright (c) 2020 udv. All rights reserved.

#include <utility>

#include "udv/todo_list.hpp"

namespace udv {

	TodoList::~TodoList() {
		mWorking = false;
		if (mRunningThread.joinable()) {
			mRunningThread.join();
		}
	}

	TodoList &TodoList::operator=(const TodoList &other) {
		if (this == &other) {
			return *this;
		}

		mItems = other.mItems;
		mWorking = other.mWorking;
		mRunningThread = std::thread{routine, std::ref(*this)};

		return *this;
	}

	TodoList &TodoList::operator=(TodoList &&other) noexcept {
		if (this == &other) {
			return *this;
		}

		mItems = std::move(other.mItems);
		mWorking = other.mWorking;
		mRunningThread = std::move(other.mRunningThread);

		return *this;
	}

	void TodoList::setCallbackNotifier(TodoList::callback_type callback) {
		std::lock_guard guard{mMutex};

		mCallback = std::move(callback);
	}

	void TodoList::detachCallback() {
		std::lock_guard guard{mMutex};

		mCallback = nullptr;
	}

	void TodoList::tryTrigger() {
		std::lock_guard guard{mMutex};
		auto item = mItems.back();

		if (NeedsToBeTriggered(item)) {
			mCallback(item);
			mItems.pop_back();
		}
	}

	void TodoList::routine(TodoList &list) {
		while (list.mWorking) {
			if (!list.empty()) {
				list.tryTrigger();
			}
		}
	}

}
