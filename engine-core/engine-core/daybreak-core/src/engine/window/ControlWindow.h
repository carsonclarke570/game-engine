#pragma once

#include "platform/win32/ComboBox.h"

namespace Daybreak {

	class ControlWindow : public win32::Window {

		public:
			ControlWindow();
			~ControlWindow();

			virtual void Paint(HDC hdc) override;
			virtual void Build() override;

			virtual LRESULT MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override;

			static std::map<std::wstring, dx12::AttachmentPoint> g_renderTargetOptions;

		private:
			std::shared_ptr<win32::ComboBox>  m_renderTargetSelect;;
	};
}