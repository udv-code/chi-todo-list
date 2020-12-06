// Copyright (c) 2020 udv. All rights reserved.

#include <utility>
#include <fstream>
#include <sstream>

#include "udv/todo_list.hpp"

namespace udv {

	TodoList::~TodoList() {
		mWorking = false;
		if (mRunningThread.joinable()) {
			mRunningThread.join();
		}

		if (mExport) {
			exportSettings();
		}

		for (auto &p : mItems) {
			delete p;
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

	void TodoList::setCallbackNotifier(const TodoList::callback_type &callback) {
		std::lock_guard guard{mMutex};

		mCallback = callback;
	}

	void TodoList::detachCallback() {
		std::lock_guard guard{mMutex};

		mCallback = nullptr;
	}

	void TodoList::tryTrigger() {
		std::lock_guard guard{mMutex};
		auto* item = mItems.back();

		if (NeedsToBeTriggered(*item)) {
			mCallback(*item);
			if (item->isPeriodic()) {
				item->triggerTime += item->interval();
				sort();
			} else {
				mItems.pop_back();
			}
		}
	}

	void TodoList::routine(TodoList &list) {
		while (list.mWorking) {
			if (!list.empty()) {
				list.tryTrigger();
			}
		}
	}

	void TodoList::exportSettings(const std::string &filename) {
		std::ofstream file;
		file.open(filename, std::ios::trunc);

		if (!file.fail()) {
			if (file.bad()) {
				return;
			}

			for (const auto &item : mItems) {
				file << *item;
			}
		}
	}

	void TodoList::importSettings(const std::string &filename) {
		std::ifstream file;
		file.open(filename);

		if (!file.fail()) {
			if (file.bad()) {
				return;
			}

			int64_t triggerEpochSecs;
			std::string message;

			std::string line;
			char divider;
			char periodChar;
			std::istringstream iss;
			while (!file.eof()) {
				line.clear();
				message.clear();
				std::getline(file, line);

				if (line.empty()) {
					continue;
				}
				iss.str(line);
				iss >> periodChar;
				iss >> triggerEpochSecs >> divider;
				iss.get();
				std::getline(iss, message);

				std::chrono::system_clock::time_point point{std::chrono::seconds{triggerEpochSecs}};
				if (periodChar == NP_CHAR) {
					addItem(point, message);
				} else {
					addItem(periodChar, point, message);
				}
			}
		}
	}

}
