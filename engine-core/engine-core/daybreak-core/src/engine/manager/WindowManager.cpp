#include "daybreak.h"
#include "WindowManager.h"

#include "engine/SplashScreen.h"
#include "engine/Simulation.h"
#include "engine/MetricsWindow.h"
#include "engine/ControlWindow.h"

namespace WindowManagerUtil {
	
	WindowManager* m_windowManager;
	Daybreak::Simulation* m_mainSimulation;

	void Build(win32::IApplication* app) {
		if (m_windowManager != nullptr) {
			Logger::error(L"WindowManager already created!\n");
			return;
		}

		m_windowManager = new WindowManager();

		m_mainSimulation = (Daybreak::Simulation*) app;
		m_mainSimulation->Settings();

		EngineMode mode = Engine::GetMode();
		if (mode == EngineMode::DEBUG || mode == EngineMode::EDITOR) {
			m_windowManager->Add(L"MetricsWindow", new Daybreak::MetricsWindow());
			m_windowManager->Add(L"ControlWindow", new Daybreak::ControlWindow());
		}
	}
	
	void Initialize() {
		if (m_windowManager == nullptr) {
			Logger::error(L"WindowManager not created yet!\n");
			return;
		}

		SplashScreen::Open();
		
		// Initialize DAYBREAK
		Logger::log_debug_seperator();
		Logger::info(L"[DAYBREAK STARTUP] Daybreak Initialization...\n");
		m_mainSimulation->PreInitialize();
		
		// Windows
		Logger::info(L"[WINDOWS STARTUP] Window Initializations...\n");
		m_windowManager->InitializeAll();
		Logger::log_debug_seperator();

		// Initialize GAME
		Logger::info(L"[GAME STARTUP] Game Initialization...\n");
		m_mainSimulation->Initialize();
		Logger::log_debug_seperator();

		SplashScreen::Close();
	}

	void Tick() {
		m_mainSimulation->Tick();
	}
	
	void Open() {
		if (m_windowManager == nullptr) {
			Logger::error(L"WindowManager not created yet!\n");
			return;
		}
		Logger::info(L"Opening Simulation window..\n");
		m_mainSimulation->Show();
		m_windowManager->OpenAll();
	}
	
	void Close() {
		if (m_windowManager == nullptr) {
			Logger::error(L"WindowManager not created yet!\n");
			return;
		}
		Logger::info(L"[GAME TEARDOWN] Shutting down...\n");
		m_mainSimulation->Teardown();

		delete m_mainSimulation;
		delete m_windowManager;
	}
}

WindowManager::WindowManager() {}

WindowManager::~WindowManager() {
	for (auto& window : m_windows) {
		delete window.second;
	}
}

void WindowManager::Add(std::wstring windowName, win32::Window* window) {
	m_windows[windowName] = window;
}

win32::Window* WindowManager::Get(std::wstring windowName) {
	return m_windows[windowName];
}

void WindowManager::Initialize(std::wstring windowName) {
	Logger::info(L"Initializing window: %s...\n", windowName.c_str());
	win32::Window* window = m_windows[windowName];
	window->RegisterNewClass();
	window->Build();
}

void WindowManager::InitializeAll() {
	for (auto &window : m_windows) {
		Initialize(window.first);
	}
}

void WindowManager::Open(std::wstring windowName) {
	Logger::info(L"Opening window: %s...\n", windowName.c_str());
	win32::Window* window = m_windows[windowName];
	window->Show();
}

void WindowManager::OpenAll() {
	for (auto &window : m_windows) {
		Open(window.first);
	}
}

