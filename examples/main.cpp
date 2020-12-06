#include <udv/todo.hpp>
#include "app.hpp"

#include <Windows.h>
#include <Pathcch.h>
#include <propvarutil.h>
#include <SDKDDKVer.h>
#include <strsafe.h>
#include <windows.ui.notifications.h>
#include <wrl.h>
#include <wrl\wrappers\corewrappers.h>

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
            PSTR lpCmdLine, INT nCmdShow) {
	using namespace Microsoft::WRL::Wrappers;

	RoInitializeWrapper winRtInitializer(RO_INIT_MULTITHREADED);
	HRESULT hr = winRtInitializer;
	if (SUCCEEDED(hr)) {
		udv::WinApp app;
		hr = app.Initialize(hInstance);
		if (SUCCEEDED(hr)) {
			app.RunMessageLoop();
		}
	}

	return SUCCEEDED(hr);

	// bool allowedToWrite = true;
	// auto callback = [&allowedToWrite](const TodoItem &item) {
	// 	while (!allowedToWrite) {
	//
	// 	}
	// 	std::cout << "Received notification: " << item.message << std::endl;
	// };
	//
	// TodoList list;
	// auto guard = list.guard(callback);
	//
	// // list.readFrom(std::cin);
	// std::string msg;
	// int offsetSecs = 0;
	// std::chrono::time_point<std::chrono::system_clock> triggerTime;
	// while (true) {
	// 	msg.clear();
	// 	allowedToWrite = false;
	// 	std::cout << "Input new message:";
	// 	std::getline(std::cin, msg);
	// 	if (msg == "exit") {
	// 		std::cout << "Are you really want to exit (Y)?";
	// 		char ans = std::cin.get();
	// 		if (ans == 'Y') {
	// 			break;
	// 		}
	// 	}
	// 	std::cout << "Input trigger time for '" << msg << "':";
	// 	std::cin >> offsetSecs;
	// 	std::cin.get();
	// 	allowedToWrite = true;
	// 	std::this_thread::sleep_for(std::chrono::seconds{1});
	//
	// 	triggerTime = std::chrono::system_clock::now() + std::chrono::seconds(offsetSecs);
	//
	// 	list.addItem(triggerTime, msg);
	// }
	//
	// return 0;
}
