#include "daybreak.h"
#include "Win32Caption.h"

namespace win32 {
	void Caption::AddCaptionButton(CaptionButton* button) {
		m_captionButtons.push_back(button);
	}
}


