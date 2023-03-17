#include "daybreak.h"

namespace Engine {

	DaybreakEngine g_engine;

	void SetMode(EngineMode mode) {
		g_engine.SetEngineMode(mode);
	}

	EngineMode GetMode(){
		return g_engine.GetEngineMode();
	}

	std::wstring DAYBREAK_API ModeToString() {
		switch (Engine::GetMode()) {
			case EngineMode::DEBUG:		return L"Debug";
			case EngineMode::RELEASE:	return L"Release";
			case EngineMode::SERVER:	return L"Server";
			case EngineMode::EDITOR:	return L"Editor";
			default:					return L"None";
		}
	}
}

DaybreakEngine::DaybreakEngine() {
#ifdef _DEBUG
	m_mode = EngineMode::DEBUG;
#else
	m_mode = EngineMode::RELEASE;
#endif 
}

DaybreakEngine::~DaybreakEngine() {}

EngineMode DaybreakEngine::GetEngineMode() {
	return m_mode;
}

void DaybreakEngine::SetEngineMode(EngineMode mode) {
	m_mode = mode;
}
