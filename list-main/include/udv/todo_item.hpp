// Copyright (c) 2020 udv. All rights reserved.

#ifndef TODO_LIST_TODO_ITEM
#define TODO_LIST_TODO_ITEM

#include <string>
#include <chrono>
#include <utility>

namespace udv {
	struct TodoItem {
		TodoItem(const std::chrono::time_point<std::chrono::system_clock> &trigger_time, std::string message)
				: triggerTime(trigger_time), message(std::move(message)) {}

		std::chrono::time_point<std::chrono::system_clock> triggerTime;
		std::string message;
	};

	struct TodoPeriodicItem : public TodoItem {
		std::chrono::duration<std::chrono::system_clock> period;
	};

	inline bool NeedsToBeTriggered(const TodoItem &item) {
		return item.triggerTime <= std::chrono::system_clock::now();
	}
}

#endif //TODO_LIST_TODO_ITEM
