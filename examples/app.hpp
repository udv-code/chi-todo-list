// Copyright (c) 2020 udv. All rights reserved.

#ifndef TODO_LIST_APP
#define TODO_LIST_APP

#include <Windows.h>
#include <Pathcch.h>
#include <propvarutil.h>
#include <string>
#include <SDKDDKVer.h>
#include <strsafe.h>
#include <windows.ui.notifications.h>
#include <wrl.h>
#include <wrl\wrappers\corewrappers.h>

#include "udv/todo_list.hpp"

namespace udv {

	class WinApp {
	public:
		static WinApp *GetInstance() {
			return sInstance;
		}

		WinApp();
		~WinApp();
		HRESULT Initialize(_In_ HINSTANCE hInstance);
		void RunMessageLoop();

		HRESULT SetStatus(std::string message);
		HRESULT ScheduleToast();

	private:
		HRESULT RegisterAppForNotificationSupport();
		HRESULT InstallShortcut(_In_ PCWSTR shortcutPath, _In_ PCWSTR exePath);
		HRESULT RegisterComServer(_In_ PCWSTR exePath);

		HRESULT RegisterActivator();
		void UnregisterActivator();

		static LRESULT CALLBACK WndProc(
				_In_ HWND hWnd,
				_In_ UINT message,
				_In_ WPARAM wParam,
				_In_ LPARAM lParam
		);

		HRESULT DisplayToast(std::string message);

		HRESULT CreateToastXml(
				_In_ ABI::Windows::UI::Notifications::IToastNotificationManagerStatics *toastManager,
				_COM_Outptr_ ABI::Windows::Data::Xml::Dom::IXmlDocument **xml,
				_In_ PCWSTR message
		);

		HRESULT CreateToast(
				_In_ ABI::Windows::UI::Notifications::IToastNotificationManagerStatics *toastManager,
				_In_ ABI::Windows::Data::Xml::Dom::IXmlDocument *xml
		);
		HRESULT SetImageSrc(
				_In_ PCWSTR imagePath,
				_In_ ABI::Windows::Data::Xml::Dom::IXmlDocument *toastXml
		);
		HRESULT SetTextValues(
				_In_reads_(textValuesCount) const PCWSTR *textValues,
				_In_ UINT32 textValuesCount,
				_Inout_ ABI::Windows::Data::Xml::Dom::IXmlDocument *toastXml
		);
		HRESULT SetNodeValueString(
				_In_ HSTRING onputString,
				_Inout_ ABI::Windows::Data::Xml::Dom::IXmlNode *node,
				_In_ ABI::Windows::Data::Xml::Dom::IXmlDocument *xml
		);

		HWND m_hwnd = nullptr;
		HWND m_hEditMessage = nullptr;
		HWND m_hEditTime = nullptr;
		HWND m_hStatus = nullptr;

		static const WORD HM_TEXTBUTTON = 1;

		static WinApp *sInstance;
		constexpr static wchar_t AppId[] = L"udv.TodoList";
	private:
		static void handleItemTrigger(TodoItem item);
	private:
		TodoList::NotifierGuard* mListGuard;
		TodoList mList;
	};
}

#endif //TODO_LIST_APP
