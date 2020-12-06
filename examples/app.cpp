// Copyright (c) 2020 udv. All rights reserved.
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "app.hpp"

#include <string>
#include <codecvt>
#include <vector>

#include <SDKDDKVer.h>
#include <Windows.h>
#include <Psapi.h>
#include <strsafe.h>
#include <ShObjIdl.h>
#include <Shlobj.h>
#include <Pathcch.h>
#include <propvarutil.h>
#include <propkey.h>
#include <wrl.h>
#include <wrl\wrappers\corewrappers.h>
#include <windows.ui.notifications.h>
#include "NotificationActivationCallback.h"
#include <sstream>

#define W_WIDTH 500
#define W_MARGIN 20
#define FIELD_HEIGHT 50
#define FIELD_MARGIN W_MARGIN / 2
#define FIELD_WIDTH W_WIDTH - W_MARGIN * 2
#define W_HEIGHT 4 * FIELD_HEIGHT + 9 * FIELD_MARGIN

namespace udv
{
	WinApp* WinApp::sInstance = nullptr;

	using namespace ABI::Windows::Data::Xml::Dom;
	using namespace ABI::Windows::UI::Notifications;
	using namespace Microsoft::WRL;

	struct CoTaskMemStringTraits
	{
		typedef PWSTR Type;

		inline static bool Close(_In_ Type h) noexcept
		{
			::CoTaskMemFree(h);
			return true;
		}

		inline static Type GetInvalidValue() noexcept { return nullptr; }
	};

	typedef Wrappers::HandleT<CoTaskMemStringTraits> CoTaskMemString;
	EXTERN_C const PROPERTYKEY DECLSPEC_SELECTANY PKEY_AppUserModel_ToastActivatorCLSID = {
		{0x9F4C2855, 0x9F79, 0x4B39, {0xA8, 0xD0, 0xE1, 0xD4, 0x2D, 0xE1, 0xD5, 0xF3}}, 26
	};

	// For the app to be activated from Action Center, it needs to provide a COM server to be called
	// when the notification is activated.  The CLSID of the object needs to be registered with the
	// OS via its shortcut so that it knows who to call later.
	class DECLSPEC_UUID("23A5B06E-20BB-4E7E-A0AC-6982ED6A6041") NotificationActivator WrlSealed
		: public RuntimeClass<RuntimeClassFlags<ClassicCom>,
		                      INotificationActivationCallback> WrlFinal {
	public:
		virtual HRESULT STDMETHODCALLTYPE Activate(
			_In_ LPCWSTR /*appUserModelId*/,
			_In_ LPCWSTR /*invokedArgs*/,
			/*_In_reads_(dataCount)*/ const NOTIFICATION_USER_INPUT_DATA* /*data*/,
			ULONG /*dataCount*/) override
		{
			return WinApp::GetInstance()->SetStatus(
				"NotificationActivator - The user clicked on the toast.");
		}
	};

	CoCreatableClass(NotificationActivator);

	WinApp::WinApp()
	{
		sInstance = this;
		mListGuard = new TodoList::NotifierGuard{mList, handleItemTrigger};
	}

	WinApp::~WinApp()
	{
		delete mListGuard;
		UnregisterActivator();
		sInstance = nullptr;
	}

	static std::wstring s2lpcw(const std::string& s)
	{
		// int len;
		// int slength = (int)s.length() + 1;
		// len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		// wchar_t* buf = new wchar_t[len];
		// MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		// std::wstring r(buf);
		// delete[] buf;
		// return r;
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::wstring wide = converter.from_bytes(s);
		return wide;
	}

	static std::string utf16ToUTF8(const std::wstring& s)
	{
		const int size = ::WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, NULL, 0, 0, NULL);

		std::vector<char> buf(size);
		::WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, &buf[0], size, 0, NULL);

		return std::string(&buf[0]);
	}

	void WinApp::handleItemTrigger(TodoItem item)
	{
		auto app = WinApp::GetInstance();
		app->SetStatus("Received Notification");
		app->DisplayToast(item.message);
	}

	HRESULT WinApp::ScheduleToast()
	{
		auto app = WinApp::GetInstance();

		std::stringstream status(std::ios_base::app | std::ios_base::in | std::ios_base::out);
		// Reading message
		std::string message;
		int len = GetWindowTextLength(app->m_hEditMessage) + 1;
		message.resize(len - 1);
		GetWindowText(app->m_hEditMessage, const_cast<char*>(message.c_str()), len);

		// Reading trigger point
		std::chrono::time_point<std::chrono::system_clock> triggerTime = std::chrono::system_clock::now() +
			std::chrono::seconds{2};

		std::string triggerPoint;
		len = GetWindowTextLength(app->m_hEditTime) + 1;
		triggerPoint.resize(len - 1);
		GetWindowText(app->m_hEditTime, const_cast<char*>(triggerPoint.c_str()), len);

		status << "Scheduled '" << message << "' On " << triggerPoint << "!";
		app->SetStatus(status.str());

		// Adding item
		app->mList.addItem(triggerTime, message);
		return S_OK;
	}


	// In order to display toasts, a desktop application must have a shortcut on the Start menu.
	// Also, an AppUserModelID must be set on that shortcut.
	//
	// For the app to be activated from Action Center, it needs to register a COM server with the OS
	// and register the CLSID of that COM server on the shortcut.
	//
	// The shortcut should be created as part of the installer. The following code shows how to create
	// a shortcut and assign the AppUserModelID and ToastActivatorCLSID properties using Windows APIs.
	//
	// Included in this project is a wxs file that be used with the WiX toolkit
	// to make an installer that creates the necessary shortcut. One or the other should be used.
	//
	// This sample doesn't clean up the shortcut or COM registration.


	HRESULT WinApp::RegisterAppForNotificationSupport()
	{
		CoTaskMemString appData;
		auto hr = ::SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, appData.GetAddressOf());
		if (SUCCEEDED(hr))
		{
			wchar_t shortcutPath[MAX_PATH];
			hr = ::PathCchCombine(shortcutPath, ARRAYSIZE(shortcutPath), appData.Get(),
			                      LR"(Microsoft\Windows\Start Menu\Programs\TodoList.lnk)");
			if (SUCCEEDED(hr))
			{
				DWORD attributes = ::GetFileAttributes(utf16ToUTF8(shortcutPath).c_str());
				bool fileExists = attributes < 0xFFFFFFF;
				if (!fileExists)
				{
					wchar_t exePath[MAX_PATH];
					DWORD charWritten = ::GetModuleFileNameW(nullptr, exePath, ARRAYSIZE(exePath));
					hr = charWritten > 0 ? S_OK : HRESULT_FROM_WIN32(::GetLastError());
					if (SUCCEEDED(hr))
					{
						hr = InstallShortcut(shortcutPath, exePath);
						if (SUCCEEDED(hr))
						{
							hr = RegisterComServer(exePath);
						}
					}
				}
			}
		}
		return hr;
	}

	_Use_decl_annotations_

	HRESULT WinApp::InstallShortcut(PCWSTR shortcutPath, PCWSTR exePath)
	{
		ComPtr<IShellLink> shellLink;
		HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&shellLink));
		if (SUCCEEDED(hr))
		{
			hr = shellLink->SetPath(utf16ToUTF8(exePath).c_str());
			if (SUCCEEDED(hr))
			{
				ComPtr<IPropertyStore> propertyStore;

				hr = shellLink.As(&propertyStore);
				if (SUCCEEDED(hr))
				{
					PROPVARIANT propVar;
					propVar.vt = VT_LPWSTR;
					propVar.pwszVal = const_cast<PWSTR>(AppId); // for _In_ scenarios, we don't need a copy
					hr = propertyStore->SetValue(PKEY_AppUserModel_ID, propVar);
					if (SUCCEEDED(hr))
					{
						propVar.vt = VT_CLSID;
						propVar.puuid = const_cast<CLSID*>(&__uuidof(NotificationActivator));
						hr = propertyStore->SetValue(PKEY_AppUserModel_ToastActivatorCLSID, propVar);
						if (SUCCEEDED(hr))
						{
							hr = propertyStore->Commit();
							if (SUCCEEDED(hr))
							{
								ComPtr<IPersistFile> persistFile;
								hr = shellLink.As(&persistFile);
								if (SUCCEEDED(hr))
								{
									hr = persistFile->Save(shortcutPath, TRUE);
								}
							}
						}
					}
				}
			}
		}
		return hr;
	}

	_Use_decl_annotations_

	HRESULT WinApp::RegisterComServer(PCWSTR exePath)
	{
		// We don't need to worry about overflow here as ::GetModuleFileName won't
		// return anything bigger than the max file system path (much fewer than max of DWORD).
		DWORD dataSize = static_cast<DWORD>((::wcslen(exePath) + 1) * sizeof(WCHAR));

		// In this sample, the app UI is registered to launch when the COM callback is needed.
		// Other options might be to launch a background process instead that then decides to launch
		// the UI if needed by that particular notification.
		return HRESULT_FROM_WIN32(::RegSetKeyValue(
			HKEY_CURRENT_USER,
			"(SOFTWARE\\Classes\\CLSID\\{23A5B06E-20BB-4E7E-A0AC-6982ED6A6041}\\LocalServer32)",
			nullptr,
			REG_SZ,
			reinterpret_cast<const BYTE*>(exePath),
			dataSize));
	}

	// Register activator for notifications
	HRESULT WinApp::RegisterActivator()
	{
		// Module<OutOfProc> needs a callback registered before it can be used.
		// Since we don't care about when it shuts down, we'll pass an empty lambda here.
		Module<OutOfProc>::Create([]
		{
		});

		// If a local server process only hosts the COM object then COM expects
		// the COM server host to shutdown when the references drop to zero.
		// Since the user might still be using the program after activating the notification,
		// we don't want to shutdown immediately.  Incrementing the object count tells COM that
		// we aren't done yet.
		Module<OutOfProc>::GetModule().IncrementObjectCount();

		return Module<OutOfProc>::GetModule().RegisterObjects();
	}

	// Unregister our activator COM object
	void WinApp::UnregisterActivator()
	{
		Module<OutOfProc>::GetModule().UnregisterObjects();

		Module<OutOfProc>::GetModule().DecrementObjectCount();
	}

	// Prepare the main window
	_Use_decl_annotations_

	HRESULT WinApp::Initialize(HINSTANCE hInstance)
	{
		HRESULT hr = RegisterAppForNotificationSupport();
		if (SUCCEEDED(hr))
		{
			WNDCLASSEX wcex = {sizeof(wcex)};
			// Register window class
			wcex.style = CS_HREDRAW | CS_VREDRAW;
			wcex.lpfnWndProc = WinApp::WndProc;
			wcex.cbWndExtra = sizeof(LONG_PTR);
			wcex.hInstance = hInstance;
			wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
			wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
			wcex.lpszClassName = "TodoList";
			wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
			::RegisterClassEx(&wcex);

			// Create window
			m_hwnd = CreateWindow(
				"TodoList",
				"TodoList",
				WS_BORDER | WS_SYSMENU,
				CW_USEDEFAULT, CW_USEDEFAULT,
				W_WIDTH, W_HEIGHT,
				nullptr, nullptr,
				hInstance, this);

			hr = m_hwnd ? S_OK : E_FAIL;
			if (SUCCEEDED(hr))
			{
				m_hStatus = ::CreateWindow(
					"EDIT",
					"",
					ES_LEFT | ES_MULTILINE | ES_READONLY | WS_CHILD | WS_VISIBLE | WS_BORDER,
					FIELD_MARGIN, FIELD_MARGIN, FIELD_WIDTH, FIELD_HEIGHT,
					m_hwnd, nullptr,
					hInstance, nullptr);
				m_hEditMessage = ::CreateWindow(
					"EDIT",
					"",
					ES_LEFT | ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_BORDER,
					FIELD_MARGIN, FIELD_MARGIN * 2 + FIELD_HEIGHT * 1, FIELD_WIDTH, FIELD_HEIGHT,
					m_hwnd, nullptr,
					hInstance, nullptr);
				m_hEditTime = ::CreateWindow(
					"EDIT",
					"",
					ES_LEFT | ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_BORDER,
					FIELD_MARGIN, FIELD_MARGIN * 3 + FIELD_HEIGHT * 2, FIELD_WIDTH, FIELD_HEIGHT,
					m_hwnd, nullptr,
					hInstance, nullptr);
				::CreateWindow(
					"BUTTON",
					"Schedule",
					BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
					FIELD_MARGIN, FIELD_MARGIN * 4 + FIELD_HEIGHT * 3, FIELD_WIDTH, FIELD_HEIGHT,
					m_hwnd, reinterpret_cast<HMENU>(HM_TEXTBUTTON),
					hInstance, nullptr);

				::ShowWindow(m_hwnd, SW_SHOWNORMAL);
				::UpdateWindow(m_hwnd);
			}
		}

		if (SUCCEEDED(hr))
		{
			hr = RegisterActivator();
		}

		return hr;
	}

	// Standard message loop
	void WinApp::RunMessageLoop()
	{
		MSG msg;
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	HRESULT WinApp::SetStatus(std::string message)
	{
		::SetForegroundWindow(m_hwnd);

		::SendMessage(m_hStatus, WM_SETTEXT, reinterpret_cast<WPARAM>(nullptr),
		              reinterpret_cast<LPARAM>(message.c_str()));

		return S_OK;
	}

	// Display the toast using classic COM. Note that is also possible to create and
	// display the toast using the new C++ /ZW options (using handles, COM wrappers, etc.)
	HRESULT WinApp::DisplayToast(std::string message)
	{
		std::wstring temp = s2lpcw(message);
		LPCWSTR message_convert = temp.c_str();

		ComPtr<IToastNotificationManagerStatics> toastStatics;
		HRESULT hr = Windows::Foundation::GetActivationFactory(
			Wrappers::HStringReference(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager).Get(),
			&toastStatics);
		if (SUCCEEDED(hr))
		{
			ComPtr<IXmlDocument> toastXml;
			hr = CreateToastXml(toastStatics.Get(), &toastXml, message_convert);
			if (SUCCEEDED(hr))
			{
				hr = CreateToast(toastStatics.Get(), toastXml.Get());
			}
		}
		return hr;
	}

	// Create the toast XML from a template
	_Use_decl_annotations_

	HRESULT WinApp::CreateToastXml(IToastNotificationManagerStatics* toastManager, IXmlDocument** inputXml,
	                               PCWSTR message)
	{
		*inputXml = nullptr;

		// Retrieve the template XML
		HRESULT hr = toastManager->GetTemplateContent(ToastTemplateType_ToastImageAndText04, inputXml);
		if (SUCCEEDED(hr))
		{
			PWSTR imagePath = _wfullpath(nullptr, L"toastImageAndText.png", MAX_PATH);
			hr = imagePath != nullptr ? S_OK : HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
			if (SUCCEEDED(hr))
			{
				hr = SetImageSrc(imagePath, *inputXml);
				if (SUCCEEDED(hr))
				{
					const PCWSTR textValues[] = {
						message
					};

					hr = SetTextValues(textValues, ARRAYSIZE(textValues), *inputXml);
				}

				free(imagePath);
			}
		}
		return hr;
	}

	// Set the value of the "src" attribute of the "image" node
	_Use_decl_annotations_

	HRESULT WinApp::SetImageSrc(PCWSTR imagePath, IXmlDocument* toastXml)
	{
		wchar_t imageSrcUri[MAX_PATH];
		DWORD size = ARRAYSIZE(imageSrcUri);

		HRESULT hr = ::UrlCreateFromPathW(imagePath, imageSrcUri, &size, 0);
		if (SUCCEEDED(hr))
		{
			ComPtr<IXmlNodeList> nodeList;
			hr = toastXml->GetElementsByTagName(Wrappers::HStringReference(L"image").Get(), &nodeList);
			if (SUCCEEDED(hr))
			{
				ComPtr<IXmlNode> imageNode;
				hr = nodeList->Item(0, &imageNode);
				if (SUCCEEDED(hr))
				{
					ComPtr<IXmlNamedNodeMap> attributes;

					hr = imageNode->get_Attributes(&attributes);
					if (SUCCEEDED(hr))
					{
						ComPtr<IXmlNode> srcAttribute;

						hr = attributes->GetNamedItem(Wrappers::HStringReference(L"src").Get(), &srcAttribute);
						if (SUCCEEDED(hr))
						{
							hr = SetNodeValueString(Wrappers::HStringReference(imageSrcUri).Get(), srcAttribute.Get(),
							                        toastXml);
						}
					}
				}
			}
		}
		return hr;
	}

	// Set the values of each of the text nodes
	_Use_decl_annotations_

	HRESULT WinApp::SetTextValues(const PCWSTR* textValues, UINT32 textValuesCount, IXmlDocument* toastXml)
	{
		ComPtr<IXmlNodeList> nodeList;
		HRESULT hr = toastXml->GetElementsByTagName(Wrappers::HStringReference(L"text").Get(), &nodeList);
		if (SUCCEEDED(hr))
		{
			UINT32 nodeListLength;
			hr = nodeList->get_Length(&nodeListLength);
			if (SUCCEEDED(hr))
			{
				// If a template was chosen with fewer text elements, also change the amount of strings
				// passed to this method.
				hr = textValuesCount <= nodeListLength ? S_OK : E_INVALIDARG;
				if (SUCCEEDED(hr))
				{
					for (UINT32 i = 0; i < textValuesCount; i++)
					{
						ComPtr<IXmlNode> textNode;
						hr = nodeList->Item(i, &textNode);
						if (SUCCEEDED(hr))
						{
							hr = SetNodeValueString(Wrappers::HStringReference(textValues[i]).Get(), textNode.Get(),
							                        toastXml);
						}
					}
				}
			}
		}

		return hr;
	}

	_Use_decl_annotations_

	HRESULT WinApp::SetNodeValueString(HSTRING inputString, IXmlNode* node, IXmlDocument* xml)
	{
		ComPtr<IXmlText> inputText;
		HRESULT hr = xml->CreateTextNode(inputString, &inputText);
		if (SUCCEEDED(hr))
		{
			ComPtr<IXmlNode> inputTextNode;
			hr = inputText.As(&inputTextNode);
			if (SUCCEEDED(hr))
			{
				ComPtr<IXmlNode> appendedChild;
				hr = node->AppendChild(inputTextNode.Get(), &appendedChild);
			}
		}

		return hr;
	}

	// Create and display the toast
	_Use_decl_annotations_

	HRESULT WinApp::CreateToast(IToastNotificationManagerStatics* toastManager, IXmlDocument* xml)
	{
		ComPtr<IToastNotifier> notifier;
		HRESULT hr = toastManager->CreateToastNotifierWithId(Wrappers::HStringReference(AppId).Get(), &notifier);
		if (SUCCEEDED(hr))
		{
			ComPtr<IToastNotificationFactory> factory;
			hr = Windows::Foundation::GetActivationFactory(
				Wrappers::HStringReference(RuntimeClass_Windows_UI_Notifications_ToastNotification).Get(),
				&factory);
			if (SUCCEEDED(hr))
			{
				ComPtr<IToastNotification> toast;
				hr = factory->CreateToastNotification(xml, &toast);
				if (SUCCEEDED(hr))
				{
					hr = notifier->Show(toast.Get());
				}
			}
		}
		return hr;
	}

	// Standard window procedure
	_Use_decl_annotations_

	LRESULT CALLBACK WinApp::WndProc(HWND hwnd, UINT32 message, WPARAM wParam, LPARAM lParam)
	{
		if (message == WM_CREATE)
		{
			auto pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
			auto app = reinterpret_cast<WinApp*>(pcs->lpCreateParams);

			SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));

			return 1;
		}

		auto app = reinterpret_cast<WinApp*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		if (app)
		{
			switch (message)
			{
			case WM_COMMAND:
				{
					int wmId = LOWORD(wParam);
					switch (wmId)
					{
					case WinApp::HM_TEXTBUTTON: app->ScheduleToast();
						break;
					default: return DefWindowProc(hwnd, message, wParam, lParam);
					}
				}
				break;
			case WM_PAINT:
				{
					PAINTSTRUCT ps;
					BeginPaint(hwnd, &ps);
					EndPaint(hwnd, &ps);
				}
				return 0;

			case WM_DESTROY:
				{
					PostQuitMessage(0);
				}
				return 1;
			}
		}
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
}
