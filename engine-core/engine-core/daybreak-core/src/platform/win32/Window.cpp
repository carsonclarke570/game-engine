#include "daybreak.h"
#include "Window.h"

#include "engine/manager/FPSCounter.h"

#define DCX_USESTYLE 0x00010000

namespace win32 {
	Window::Window(std::wstring title, HICON icon, WindowType type)
		: win32::SubObject(title, title, icon), m_type(type) {
		SetSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
	}

	Window::~Window() {
	}

	void Window::Build() {
		RECT desktop;
		const HWND desktopHandle = GetDesktopWindow();
		GetWindowRect(desktopHandle, &desktop);

		RECT r = { 0, 0, m_size.cx, m_size.cy };
		AdjustWindowRect(&r, m_type, false);
		int width = r.right - r.left;
		int height = r.bottom - r.top;

		m_handle = CreateWindow(m_class.c_str(), m_title.c_str(),
			m_type, ((desktop.right / 2) - (m_size.cx / 2)), ((desktop.bottom / 2) - (m_size.cy / 2)), m_size.cx, m_size.cy, 0, 0, HInstance(), (void*)this);
	}

	void Window::Show() {
		ShowWindow(Handle(), SW_SHOW);
		UpdateWindow(Handle());
	}

	LRESULT Window::MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		Logger::debug(L"Window handler: %d\n", message);
		switch (message) {
			case WM_NCCREATE: { OnNonClientCreate(); }									return true;
			case WM_NCACTIVATE: { OnNonClientActivate(LOWORD(wParam) != WA_INACTIVE); } return true;
			case WM_NCPAINT: { OnNonClientPaint((HRGN) wParam); }						return false;
			case WM_NCLBUTTONDOWN: { OnNonClientLeftMouseButtonDown(); }				break;
			case WM_NCLBUTTONDBLCLK: { win32::util::MaximizeWindow(Handle()); }			return false;

			case WM_GETMINMAXINFO: { OnGetMinMaxInfo((MINMAXINFO*)lParam);  }			return false;
			case WM_EXITSIZEMOVE: { OnExitSizeMove(); }									break;
			case WM_TIMER: { Redraw(); }												break;

			case WM_PAINT: { OnPaint(); }												break;
			case WM_CLOSE: { OnClose(); }												break;
			case WM_DESTROY: { OnDestroy(); }											break;
		}
		return SubObject::MessageHandler(hwnd, message, wParam, lParam);
	}

	void Window::Paint(HDC hdc) {}

	void Window::Redraw() {
		SetWindowPos(Handle(), 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_DRAWFRAME | SWP_FRAMECHANGED);
		SendMessage(Handle(), WM_PAINT, 0, 0);
		RedrawWindow(Handle(), nullptr, nullptr, RDW_INVALIDATE);
	}

	void Window::OnNonClientCreate() {
		SetTimer(Handle(), 1, 100, nullptr);
		SetWindowTheme(Handle(), L"", L"");

		win32::util::ModifyClassStyle(Handle(), 0, CS_DROPSHADOW);
		win32::Caption::AddCaptionButton(new CaptionButton(L"\U0001F5D9", CB_CLOSE, 50));		// close
		win32::Caption::AddCaptionButton(new CaptionButton(L"\U0001F5D6", CB_MAXIMIZE, 50));	// maximize
		win32::Caption::AddCaptionButton(new CaptionButton(L"\U0001F5D5", CB_MINIMIZE, 50));	// minimize
	}

	void Window::OnNonClientActivate(bool active) {
		SetActive(active);
	}

	void Window::OnNonClientPaint(HRGN region) {
		
		// Start draw
		HDC hdc = GetDCEx(Handle(), region, DCX_WINDOW | DCX_INTERSECTRGN | DCX_USESTYLE);

		RECT rect;
		GetWindowRect(Handle(), &rect);

		SIZE size = SIZE{ rect.right - rect.left, rect.bottom - rect.top };
		RECT newRect = RECT{ 0, 0, size.cx, size.cy };
		HBITMAP hbmMem = CreateCompatibleBitmap(hdc, size.cx, size.cy);
		HANDLE oldHandle = SelectObject(hdc, hbmMem);

		// Draw
		HBRUSH brush = CreateSolidBrush(RGB(46, 46, 46));
		FillRect(hdc, &newRect, brush);
		DeleteObject(brush);

		if (IsActive() && !win32::util::IsWindowFullscreen(Handle())) {
			brush = CreateSolidBrush(RGB(178, 106, 0));
			FrameRect(hdc, &newRect, brush);
			DeleteObject(brush);
		}
		PaintCaption(hdc);
		
		// End draw
		BitBlt(hdc, 0, 0, size.cx, size.cy, hdc, 0, 0, SRCCOPY);
		SelectObject(hdc, oldHandle);
		DeleteObject(hbmMem);

		ReleaseDC(Handle(), hdc);
	}

	void Window::PaintCaption(HDC hdc) {
		RECT rect;
		GetWindowRect(Handle(), &rect);

		SIZE size = SIZE{ rect.right - rect.left, rect.bottom - rect.top };
		if (ShowTitle()) {
			rect = RECT{ 0, 0, size.cx, 30 };

			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, IsActive() ? RGB(255, 255, 255) : RGB(92, 92, 92));

			DrawText(hdc, m_title.c_str(), wcslen(m_title.c_str()), &rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
		}
		
		POINT cursorPos;
		GetCursorPos(&cursorPos);

		GetWindowRect(Handle(), &rect);
		cursorPos.x -= rect.left;
		cursorPos.y -= rect.top;

		int spacing = 0;
		for (auto& button : Caption::CaptionButtons()) {
			spacing += button->Width;
			button->Rect = RECT{ size.cx - spacing, 0, size.cx - spacing + button->Width, 30 };

			if (button->Rect.left < cursorPos.x && button->Rect.right > cursorPos.x && button->Rect.top < cursorPos.y && button->Rect.bottom > cursorPos.y) {
				HBRUSH brush = CreateSolidBrush(RGB(92, 92, 92));
				FillRect(hdc, &button->Rect, brush);
				DeleteObject(brush);
			}

			if (button->Text.compare(L"\U0001F5D6") == 0 && win32::util::IsWindowFullscreen(Handle())) {
				button->Text = L"\U0001F5D7";
			}
			else if (button->Text.compare(L"\U0001F5D7") == 0 && !win32::util::IsWindowFullscreen(Handle())) {
				button->Text = L"\U0001F5D6";
			}

			DrawText(hdc, button->Text.c_str(), wcslen(button->Text.c_str()), &button->Rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
		}		
	}

	void Window::OnNonClientLeftMouseButtonDown() {
		POINT cursorPos;
		GetCursorPos(&cursorPos);

		RECT rect;
		GetWindowRect(Handle(), &rect);
		cursorPos.x -= rect.left;
		cursorPos.y -= rect.top;

		for (auto& button : Caption::CaptionButtons()) {
			
			if (button->Rect.left < cursorPos.x && button->Rect.right > cursorPos.x && button->Rect.top < cursorPos.y && button->Rect.bottom > cursorPos.y) {
				
				switch (button->Command) {
					case CB_CLOSE:	{ SendMessage(Handle(), WM_CLOSE, 0, 0); } break;
					case CB_MINIMIZE: { ShowWindow(Handle(), SW_MINIMIZE); } break;
					case CB_MAXIMIZE: { win32::util::MaximizeWindow(Handle()); } break;
				}
				
			}
		}
	}

	void Window::OnGetMinMaxInfo(MINMAXINFO* minmax) {
		RECT WorkArea; SystemParametersInfo(SPI_GETWORKAREA, 0, &WorkArea, 0);
		minmax->ptMaxSize.x = (WorkArea.right - WorkArea.left);
		minmax->ptMaxSize.y = (WorkArea.bottom - WorkArea.top);
		minmax->ptMaxPosition.x = WorkArea.left;
		minmax->ptMaxPosition.y = WorkArea.top;
		minmax->ptMinTrackSize.x = 400;
		minmax->ptMinTrackSize.y = 300;
	}

	void Window::OnExitSizeMove() {
		RECT rect;
		GetWindowRect(Handle(), &rect);
		
		RECT workArea;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
		if (rect.top < workArea.top + 10 && !win32::util::IsWindowFullscreen(Handle())) {
			win32::util::MaximizeWindow(Handle());
		}
	}

	void Window::OnPaint() {
		// Begin Paint
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(Handle(), &ps);

		RECT rc;
		GetClientRect(Handle(), &rc);
		// Logger::info(L"Window Size: %d %d %d %d\n", rc.left, rc.top, rc.right, rc.bottom);

		// Paint
		HBRUSH brush = CreateSolidBrush(RGB(36, 36, 36));
		FillRect(hdc, &rc, brush);
		DeleteObject(brush);

		Paint(hdc);

		// End Paint
		EndPaint(Handle(), &ps);
	}

	void Window::OnClose() {
		// DestroyWindow(Handle());
	}

	void Window::OnDestroy() {
		// PostQuitMessage(0);
	}
}

