#include "daybreak.h"
#include "SplashScreen.h"

#include "platform/win32/Win32Utils.h"

namespace SplashScreen {

#define WM_OUTPUTMESSAGE (WM_USER + 0x0001)

	SplashWindow* m_SplashWindow;

	void Open() {
		if (m_SplashWindow != nullptr) {
			return;
		}
		m_SplashWindow = new SplashWindow();
	}

	void Close() {
		return void DAYBREAK_API();
	}

	void AddMessage(const wchar_t* message) {
		if (m_SplashWindow != nullptr) {
			PostMessage(m_SplashWindow->Handle(), WM_OUTPUTMESSAGE, (WPARAM)message, 0);
		} else {
			Logger::error(L"Call to AddMessage() before SplashScreen created!");
		}
	}
}

SplashWindow::SplashWindow()
	: win32::Window(L"SplashScreen", nullptr, win32::WindowType::POPUP) {
	wcscpy_s(m_outputMessage, L"Starting SplashScreen...");
	SetSize(500, 600);
	win32::Window::RegisterNewClass();
	win32::Window::Initialize();
}

SplashWindow::~SplashWindow() {
}

LRESULT SplashWindow::MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_PAINT: {
			HBITMAP bitmpa;
			HDC hdc, hmemdc;
			PAINTSTRUCT ps;

			hdc = BeginPaint(hwnd, &ps);

			win32::util::AddBitmap(GameSettings::SplashURL(), hdc, 0, 0);
			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, RGB(255, 255, 255));
			if (Engine::GetMode() != EngineMode::RELEASE) {
				std::wstring engineModeText = Engine::ModeToString() + L" Mode";
				SetTextAlign(hdc, TA_RIGHT);
				TextOut(hdc, m_size.cx - 15, 15, engineModeText.c_str(), wcslen(engineModeText.c_str()));
			}

			SetTextAlign(hdc, TA_CENTER);
			TextOut(hdc, m_size.cx / 2, m_size.cy - 30, m_outputMessage, wcslen(m_outputMessage));

			EndPaint(hwnd, &ps);
		}
		break;

		case WM_OUTPUTMESSAGE: {
			wchar_t* msg = (wchar_t*) wParam;
			wcscpy_s(m_outputMessage, msg);
			RedrawWindow();
			return 0;
		}
	}

	return Window::MessageHandler(hwnd, message, wParam, lParam);
}
