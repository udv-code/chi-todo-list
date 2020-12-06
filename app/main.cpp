#include <udv/todo.hpp>
#include "app.hpp"

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
}
