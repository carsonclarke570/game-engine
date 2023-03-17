#include "daybreak.h"
#include "CmdLineArgs.h"

#include <algorithm>

void CmdLine::ReadArguments() {
	int argc = 0;
	wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	for (int i = 1; i < argc; i++) {
		std::wstring key = argv[i];
		if (key[0] == '-') {
			key.erase(0, 1);
			std::transform(key.begin(), key.end(), key.begin(), ::tolower);
			ReadArgument(key.c_str());
		}
	}
}

void CmdLine::ReadArgument(const wchar_t* argument) {
	if (wcscmp(argument, L"mtail") == 0) {
		Logger::StartMTail();
	}
	if (wcscmp(argument, L"debug") == 0) {
		Engine::SetMode(Engine::EngineMode::DEBUG);
	}
	if (wcscmp(argument, L"editor") == 0) {
		Engine::SetMode(Engine::EngineMode::EDITOR);
	}
	if (wcscmp(argument, L"server") == 0) {
		Engine::SetMode(Engine::EngineMode::SERVER);
	}	
}
