#pragma once

namespace Daybreak {

	class MetricsWindow : public win32::Window {

	public:
		MetricsWindow();
		~MetricsWindow();

		virtual void Paint(HDC hdc) override;
	};
}
