#pragma once

#define CB_CLOSE	0
#define CB_MAXIMIZE 1
#define CB_MINIMIZE 2

namespace win32 {
	class Caption {
		public:
			struct CaptionButton {
				std::wstring Text = L"X";

				int Command;
				int Width;
				RECT Rect;

				CaptionButton(std::wstring text, int command, int width = 50) {
					Text = text;
					Width = width;
					Command = command;
				}
			};

			bool ShowTitle() { return m_showTitle; }
			void SetShowTitle(bool showTitle) { m_showTitle = showTitle; }

			void AddCaptionButton(CaptionButton* button);
			std::list<CaptionButton*> CaptionButtons() { return m_captionButtons; }

		private:
			bool m_showTitle = true;
			std::list<CaptionButton*> m_captionButtons;
	};
}

