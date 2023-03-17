#pragma once

namespace Daybreak {

	class DAYBREAK_API Simulation : public win32::IApplication, public win32::Window {
		private:
		public:
			Simulation();
			~Simulation();
	
			virtual void PreInitialize() override;
			virtual LRESULT MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) override;
	};
}

