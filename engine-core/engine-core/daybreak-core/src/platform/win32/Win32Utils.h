#pragma once 

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600

namespace win32 {

	enum WindowType : DWORD {
		STATIC		= WS_OVERLAPPED,
		RESIZABLE	= WS_SIZEBOX,
		POPUP		= WS_POPUP
	};

	namespace util {

		bool DAYBREAK_API AddBitmap(const wchar_t* szFileName, HDC hWinDC, int x = 0, int y = 0);

        inline void DAYBREAK_API ModifyWindowStyle(HWND hWnd, DWORD flagsToDisable, DWORD flagsToEnable) {
            DWORD style = GetWindowLong(hWnd, GWL_STYLE);
            SetWindowLong(hWnd, GWL_STYLE, (style & ~flagsToDisable) | flagsToEnable);
        }

        inline void DAYBREAK_API ModifyWindowExStyle(HWND hWnd, DWORD flagsToDisable, DWORD flagsToEnable) {
            DWORD exStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
            SetWindowLong(hWnd, GWL_EXSTYLE, (exStyle & ~flagsToDisable) | flagsToEnable);
        }

        inline bool DAYBREAK_API HasStyle(HWND hwnd, DWORD style) {
            DWORD dwStyle = (DWORD)GetWindowLong(hwnd, GWL_STYLE);
            return ((dwStyle & style) != 0);
        }

        inline void DAYBREAK_API ModifyClassStyle(HWND hWnd, DWORD flagsToDisable, DWORD flagsToEnable) {
            DWORD style = GetWindowLong(hWnd, GCL_STYLE);
            SetClassLong(hWnd, GCL_STYLE, (style & ~flagsToDisable) | flagsToEnable);
        }

        inline bool DAYBREAK_API IsWindowFullscreen(HWND hWnd) {
            WINDOWPLACEMENT placement;
            GetWindowPlacement(hWnd, &placement);
            return (placement.showCmd == SW_MAXIMIZE);
        }

        inline void DAYBREAK_API MaximizeWindow(HWND hwnd) {
            WINDOWPLACEMENT wPos;
            GetWindowPlacement(hwnd, &wPos);
            if (wPos.showCmd == SW_MAXIMIZE) {
                ShowWindow(hwnd, SW_NORMAL);
            } else {
                ShowWindow(hwnd, SW_MAXIMIZE);
            }
        }
	}
}