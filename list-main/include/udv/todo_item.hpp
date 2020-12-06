// Copyright (c) 2020 udv. All rights reserved.

#ifndef TODO_LIST_TODO_ITEM
#define TODO_LIST_TODO_ITEM

#include <string>
#include <chrono>
#include <utility>
#include <ostream>

#define DIVIDER " \\ "

namespace udv {
	struct TodoItem {
		TodoItem(const std::chrono::time_point<std::chrono::system_clock> &trigger_time, std::string message)
				: triggerTime(trigger_time), message(std::move(message)) {}

		std::chrono::time_point<std::chrono::system_clock> triggerTime;
		std::string message;

		friend std::ostream &operator<<(std::ostream &os, const TodoItem &item) {
			os << std::chrono::duration_cast<std::chrono::milliseconds>(item.triggerTime.time_since_epoch()).count()
			   << DIVIDER << item.message << std::endl;
			return os;
		}
	};

	struct TodoPeriodicItem : public TodoItem {
		std::chrono::duration<long, std::micro> period;

		friend std::ostream &operator<<(std::ostream &os, const TodoPeriodicItem &item) {
			os << std::chrono::duration_cast<std::chrono::milliseconds>(item.triggerTime.time_since_epoch()).count()
			   << DIVIDER << item.message << DIVIDER
			   << std::chrono::duration_cast<std::chrono::milliseconds>(item.period).count()
			   << std::endl;
			return os;
		}
	};

	inline bool NeedsToBeTriggered(const TodoItem &item) {
		return item.triggerTime <= std::chrono::system_clock::now();
	}
}

#endif //TODO_LIST_TODO_ITEM
