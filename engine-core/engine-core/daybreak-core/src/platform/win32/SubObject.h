#pragma once

namespace win32 {

	class DAYBREAK_API SubObject {

		public:
			SubObject(std::wstring className, std::wstring classTitle, HICON icon);
			virtual ~SubObject();

			virtual void RegisterNewClass();
			virtual void Build() = 0;

			HWND Handle() { return m_handle; }
			void SetHandle(HWND handle) { m_handle = handle; }

		protected:
			std::wstring m_class;
			std::wstring m_title;
			HICON m_icon;
			HWND m_handle;

			static LRESULT CALLBACK	SetupMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
			static LRESULT AssignMessageHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

			virtual	LRESULT	MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	};
}