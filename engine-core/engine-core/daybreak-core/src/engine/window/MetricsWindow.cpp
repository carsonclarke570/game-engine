#include "daybreak.h"
#include "MetricsWindow.h"

#include "engine/FPSCounter.h"

namespace Daybreak {

	MetricsWindow::MetricsWindow() : win32::Window(L"Metrics Window", nullptr) {
		SetSize(400, 400);
	}
	
	MetricsWindow::~MetricsWindow() {
	}
	
	void MetricsWindow::Paint(HDC hdc) {
		// Logger::info(L"Window Size: %d %d %d %d\n", clientRect->left, clientRect->top, clientRect->right, clientRect->bottom);
		RECT clientRect;
		GetClientRect(Handle(), &clientRect);
		int middle = (clientRect.right - clientRect.left) / 2;
		RECT textRect = { 
			clientRect.left + 30, 
			clientRect.top, 
			middle,
			clientRect.top + 30 
		};
		
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(255, 255, 255));
		DrawText(hdc, L"FPS: ", -1, &textRect, DT_LEFT | DT_NOCLIP | DT_SINGLELINE | DT_VCENTER);
	

		std::wstring fps = std::to_wstring(FPSCounter::FPS());
		textRect = {
			middle,
			clientRect.top,
			clientRect.right,
			clientRect.top + 30
		};
		DrawText(hdc, fps.c_str(), -1, &textRect, DT_CENTER | DT_NOCLIP | DT_SINGLELINE | DT_VCENTER);
		SetBkMode(hdc, OPAQUE);

		// Total Time
		textRect = {
			clientRect.left + 30,
			clientRect.top + 30,
			middle,
			clientRect.top + 60
		};

		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(255, 255, 255));
		DrawText(hdc, L"Total Time (s): ", -1, &textRect, DT_LEFT | DT_NOCLIP | DT_SINGLELINE | DT_VCENTER);


		std::wstring totalTime = std::to_wstring(FPSCounter::TotalTime());
		textRect = {
			middle,
			clientRect.top + 30,
			clientRect.right,
			clientRect.top + 60
		};
		DrawText(hdc, totalTime.c_str(), -1, &textRect, DT_CENTER | DT_NOCLIP | DT_SINGLELINE | DT_VCENTER);
		SetBkMode(hdc, OPAQUE);
	}
}
