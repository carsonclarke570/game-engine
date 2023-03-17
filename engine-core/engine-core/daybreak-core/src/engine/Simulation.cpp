#include "daybreak.h"
#include "Simulation.h"
#include "engine/SplashScreen.h"

namespace Daybreak {
	
	Simulation::Simulation()
		: win32::Window(L"MainApplication", nullptr) {
	}

	Simulation::~Simulation() {}

	void Simulation::PreInitialize() {
		Logger::log_debug_seperator();
		Logger::info(L"Application starting...\n");
		Logger::info(L"Game Name: %s\n", GameSettings::game_name());
		Logger::info(L"Boot Time: %s\n", Time::get_datetime().c_str());
		Logger::info(L"Engine Mode: %s\n", Engine::ModeToString().c_str());
		Logger::log_debug_seperator();

		SplashScreen::Open();

		win32::Window::RegisterNewClass();
		win32::Window::Initialize();
	}


	LRESULT Simulation::MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		switch (message) {

		}
		return Window::MessageHandler(hwnd, message, wParam, lParam);
	}
}

