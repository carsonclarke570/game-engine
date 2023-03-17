#include "daybreak.h"

#include "IApplication.h"
#include "input/InputManager.h"
#include "common/CmdLineArgs.h"

extern win32::IApplication* entry_point();

int CALLBACK WinMain(HINSTANCE, HINSTANCE, LPSTR, INT) {
	
	auto entry_application = entry_point();

	GameSettings settings;
	entry_application->settings();

	CmdLine::ReadArguments();

	Logger logger;

	entry_application->PreInitialize();
	entry_application->initialize();

	InputManager& input_manager = InputManagerFactory::create();
	while (!input_manager.handle_exit()) {
		input_manager.handle_input();

		entry_application->update();
	}

	return 0;
} 