#include <chrono>
#include <thread>
#include <functional>
#include <iostream>

#include <udv/todo.hpp>

int main()
{
    using namespace udv;

    bool notified = false;
    auto testNotification =
        [&notified](std::string const& message) mutable
        { notified = true; std::cout << message << std::endl; };

    TodoList list;
    // TodoList::NotifierGuard guard{ list, testNotification };
    list.addItem(
        TodoItem{ std::chrono::system_clock::now() + std::chrono::seconds{ 5 }
            , "test message"});
    if(notified) {
        std::cout << "Failed. Notification has come too early";
    }
    std::this_thread::sleep_for(std::chrono::seconds{ 10 });
    if(!notified) {
        std::cout << "Failed. Notification hasn't come at all";
    }
    return 0;
}