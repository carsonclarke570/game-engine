#pragma once

#include "SubObject.h"

#include <Uxtheme.h>
#pragma comment(lib, "uxtheme.lib")


namespace win32 {

	class DAYBREAK_API Window : public win32::SubObject, public win32::Caption {
		public:
			Window(std::wstring title, HICON icon, WindowType type = RESIZABLE);
			~Window();

			virtual void Initialize() override;
			virtual LRESULT MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override;

			void RedrawWindow();
			void OnNonClientCreate();
			void OnNonClientActivate(bool active);
			void OnNonClientPaint(HRGN region);
			void PaintCaption(HDC hdc);
			void OnNonClientLeftMouseButtonDown();
			void OnGetMinMaxInfo(MINMAXINFO* minmax);
			void OnExitSizeMove();
			void OnPaint();

			SIZE Size() { return m_size; }
			void SetSize(SIZE size) { m_size = size; }
			void SetSize(int cx, int cy) { m_size.cx = cx; m_size.cy = cy; }

			bool IsActive() { return m_active; }
			void SetActive(bool active) { m_active = active; }

		protected:
			SIZE m_size;
			WindowType m_type;
			bool m_active;
	};
}