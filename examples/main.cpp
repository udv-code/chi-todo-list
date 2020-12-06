#include <Windows.h>

#include <chrono>
#include <thread>
#include <iostream>
#include <sstream>

#include <udv/todo.hpp>

class WinApp {

};

INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
            PSTR lpCmdLine, INT nCmdShow) {
	using namespace udv;

	bool allowedToWrite = true;
	auto callback = [&allowedToWrite](const TodoItem &item) {
		while (!allowedToWrite) {

		}
		std::cout << "Received notification: " << item.message << std::endl;
	};

	TodoList list;
	auto guard = list.guard(callback);

	// list.readFrom(std::cin);
	std::string msg;
	int offsetSecs = 0;
	std::chrono::time_point<std::chrono::system_clock> triggerTime;
	while (true) {
		msg.clear();
		allowedToWrite = false;
		std::cout << "Input new message:";
		std::getline(std::cin, msg);
		if (msg == "exit") {
			std::cout << "Are you really want to exit (Y)?";
			char ans = std::cin.get();
			if (ans == 'Y') {
				break;
			}
		}
		std::cout << "Input trigger time for '" << msg << "':";
		std::cin >> offsetSecs;
		std::cin.get();
		allowedToWrite = true;
		std::this_thread::sleep_for(std::chrono::seconds{1});

		triggerTime = std::chrono::system_clock::now() + std::chrono::seconds(offsetSecs);

		list.addItem(triggerTime, msg);
	}

	return 0;
}

HRESULT WinApp::SetMessage(PCWSTR message)
{
	::SetForegroundWindow(m_hwnd);

	::SendMessage(m_hEdit, WM_SETTEXT, reinterpret_cast<WPARAM>(nullptr), reinterpret_cast<LPARAM>(message));

	return S_OK;
}