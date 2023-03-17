#pragma once

#include <string>

class DAYBREAK_API DaybreakEngine;

namespace Engine {

	enum EngineMode : int {
		NONE,
		DEBUG,
		RELEASE,
		EDITOR,
		SERVER
	};

	extern DaybreakEngine g_engine;

	void DAYBREAK_API SetMode(EngineMode mode);
	EngineMode DAYBREAK_API GetMode();

	std::wstring DAYBREAK_API ModeToString();
}


using namespace Engine;
class DAYBREAK_API DaybreakEngine {
	
	private:
		EngineMode m_mode;

	public:
		DaybreakEngine();
		~DaybreakEngine();

		EngineMode GetEngineMode();
		void SetEngineMode(EngineMode mode);
};