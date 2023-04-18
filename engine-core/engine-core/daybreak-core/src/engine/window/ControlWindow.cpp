#include "daybreak.h"
#include "ControlWindow.h"

#include "engine/RenderStateManager.h"

namespace Daybreak {

	std::map<std::wstring, dx12::AttachmentPoint> ControlWindow::g_renderTargetOptions = {
		{L"DIFFUSE BUFFER", dx12::AttachmentPoint::COLOR_0 },
		{L"NORMAL BUFFER", dx12::AttachmentPoint::COLOR_1 },
		{L"POSITION BUFFER", dx12::AttachmentPoint::COLOR_2 }
	};

	ControlWindow::ControlWindow() : win32::Window(L"Controls Window", nullptr) {
		SetSize(400, 400);
	}

	ControlWindow::~ControlWindow() {}

	void ControlWindow::Paint(HDC hdc) {
		RECT clientRect;
		GetClientRect(Handle(), &clientRect);
		int middle = (clientRect.right - clientRect.left) / 2;

		RECT textRect = {
			clientRect.left + 5,
			clientRect.top,
			middle,
			clientRect.top + 30
		};

		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(255, 255, 255));
		DrawText(hdc, L"Render Target", -1, &textRect, DT_LEFT | DT_NOCLIP | DT_SINGLELINE | DT_VCENTER);
	}

	void ControlWindow::Build() {
		win32::Window::Build();

		SIZE box = { 170, 80 };
		std::vector<std::wstring> options;
		for (auto entry : g_renderTargetOptions) {
			options.push_back(entry.first);
		}

		m_renderTargetSelect = std::make_shared<win32::ComboBox>(
			L"Render Target Control",
			box, 200, 5, Handle(), options
		);
		m_renderTargetSelect->Build();
		m_renderTargetSelect->AddOptions();
	}

	LRESULT ControlWindow::MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {
			case WM_COMMAND: {
				if (HIWORD(wParam) == CBN_SELCHANGE) {
					int index = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);

					wchar_t item[256];
					SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT, (WPARAM)index, (LPARAM)item);

					std::wstring itemStr(item);
					dx12::AttachmentPoint attachment = g_renderTargetOptions[itemStr];
					RenderStateManager::SetCurrentAttachment(attachment);
					Logger::info(L"[ControlWindow] Changed render target to %d - %s\n", attachment, itemStr.c_str());
				}
			}
			return 0;
		}
		
		return win32::Window::MessageHandler(hwnd, message, wParam, lParam);
	}
}