// Copyright (c) 2020 udv. All rights reserved.

#ifndef TODO_LIST_TODO_ITEM
#define TODO_LIST_TODO_ITEM

#include <string>
#include <chrono>
#include <utility>
#include <ostream>

#define MIN_SECS 60
#define HOUR_SECS MIN_SECS * 60
#define DAY_SECS HOUR_SECS * 24
#define WEEK_SECS DAY_SECS * 7
#define MONTH_SECS WEEK_SECS * 4

#define DIVIDER " \\ "
#define NP_CHAR 'i'

namespace udv {
	// TODO: improve this classes :)

	struct TodoItem {
		TodoItem(const std::chrono::time_point<std::chrono::system_clock> &trigger_time, std::string message)
				: triggerTime(trigger_time), message(std::move(message)) {}

		std::chrono::time_point<std::chrono::system_clock> triggerTime;
		std::string message;

		virtual bool isPeriodic() const { return false; }
		virtual std::chrono::seconds interval() const { return std::chrono::seconds{0}; }

		friend std::ostream &operator<<(std::ostream &os, const TodoItem &item) {
			os << NP_CHAR << ' '
			   << std::chrono::duration_cast<std::chrono::seconds>(item.triggerTime.time_since_epoch()).count()
			   << DIVIDER << item.message
			   << std::endl;
			return os;
		}
	};

	struct TodoPeriodicItem : public TodoItem {
		TodoPeriodicItem(char period,
		                 const std::chrono::time_point<std::chrono::system_clock> &trigger_time, std::string message)
				: TodoItem(trigger_time, std::move(message)), period{period} {}

		char period;

		virtual bool isPeriodic() const override { return true; }

		virtual std::chrono::seconds interval() const override {
			switch (period) {
				case 'm': return std::chrono::seconds{MIN_SECS};
				case 'H': return std::chrono::seconds{HOUR_SECS};
				case 'W': return std::chrono::seconds{WEEK_SECS};
				case 'M': return std::chrono::seconds{MONTH_SECS};
				default: return std::chrono::seconds{0};
			}
		}

		friend std::ostream &operator<<(std::ostream &os, const TodoPeriodicItem &item) {
			os << item.period << ' '
			   << std::chrono::duration_cast<std::chrono::seconds>(item.triggerTime.time_since_epoch()).count()
			   << DIVIDER << item.message
			   << std::endl;
			return os;
		}
	};

	inline bool NeedsToBeTriggered(const TodoItem &item) {
		return item.triggerTime <= std::chrono::system_clock::now();
	}
}

#endif //TODO_LIST_TODO_ITEM
