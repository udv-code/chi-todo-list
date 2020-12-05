#include <chrono>
#include <thread>
#include <iostream>

#include <udv/todo.hpp>

int main() {
	using namespace udv;

	bool notified = false;
	auto testNotification =
			[&notified](const TodoItem &item) mutable {
				notified = true;
				std::cout << "Received notification: " << item.message << std::endl;
			};

	TodoList list;
	auto guard = list.guard(testNotification);
	list.addItem(std::chrono::system_clock::now() + std::chrono::seconds{0}, "first test message");
	list.addItem(std::chrono::system_clock::now() + std::chrono::seconds{5}, "test message");
	list.addItem(std::chrono::system_clock::now() + std::chrono::seconds{6}, "new test message");
	list.addItem(std::chrono::system_clock::now() + std::chrono::seconds{11}, "non test message");
	if (notified) {
		std::cout << "Failed. Notification has come too early";
	}
	std::this_thread::sleep_for(std::chrono::seconds{10});
	if (!notified) {
		std::cout << "Failed. Notification hasn't come at all";
	}
	return 0;
}