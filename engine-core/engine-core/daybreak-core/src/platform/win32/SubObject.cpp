#include "daybreak.h"
#include "SubObject.h"

namespace win32 {
	SubObject::SubObject(std::wstring className, std::wstring classTitle, HICON icon) 
		: m_class(className), m_title(classTitle), m_icon(icon), m_handle(nullptr) {
		
	}

	SubObject::~SubObject() {}
	
	void SubObject::RegisterNewClass() {
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;

		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(36, 36, 36));

		wcex.hIcon = m_icon;
		wcex.hIconSm = m_icon;

		wcex.lpszClassName = m_class.c_str();
		wcex.lpszMenuName = nullptr;
		wcex.hInstance = HInstance();
		wcex.lpfnWndProc = SetupMessageHandler;

		RegisterClassEx(&wcex);
	}

	LRESULT SubObject::SetupMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		Logger::debug(L"Setup with command: %d\n", msg);
		if (msg == WM_NCCREATE) {
			const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
			win32::SubObject* const pWnd = static_cast<win32::SubObject*>(pCreate->lpCreateParams);

			// SetProp(hWnd, TEXT("WindowClass"), (HANDLE) pWnd);
			//SetProp(hWnd, TEXT("WindowProcess"), (HANDLE) &win32::SubObject::AssignMessageHandler);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&win32::SubObject::AssignMessageHandler));
			pWnd->SetHandle(hWnd);

			return pWnd->MessageHandler(hWnd, msg, wParam, lParam);
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	LRESULT SubObject::AssignMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
		win32::SubObject* const pWnd = reinterpret_cast<win32::SubObject*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		Logger::debug(L"Assigning handler with: %d\n", msg);
		return pWnd->MessageHandler(hWnd, msg, wParam, lParam);
	}

	LRESULT SubObject::MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
}

