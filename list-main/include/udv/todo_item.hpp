// Copyright (c) 2020 udv. All rights reserved.

#ifndef TODO_LIST_TODO_ITEM
#define TODO_LIST_TODO_ITEM

#include <string>
#include <chrono>

namespace udv {
	struct TodoItem {
		std::chrono::time_point<std::chrono::system_clock> triggerTime;
		std::string message;
	};
}

#endif //TODO_LIST_TODO_ITEM
