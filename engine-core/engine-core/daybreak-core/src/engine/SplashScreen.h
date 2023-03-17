#pragma once

#include "platform/win32/Window.h"

namespace SplashScreen {

	void DAYBREAK_API Open();
	void DAYBREAK_API Close();
	void DAYBREAK_API AddMessage(const wchar_t* message);
}

class DAYBREAK_API SplashWindow : public win32::Window {
	public:
		SplashWindow();
		~SplashWindow();

		virtual	LRESULT	MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override;

	private:
		wchar_t m_outputMessage[MAX_NAME_STRING];
};