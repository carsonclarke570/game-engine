#include "daybreak.h"
#include "Simulation.h"
#include "engine/window/SplashScreen.h"
#include "engine/manager/FPSCounter.h"
#include "platform/dx12/CommandList.h"

namespace Daybreak {
	
	Simulation::Simulation()
		: win32::Window(L"MainApplication", nullptr),
		m_clock() {
		m_currentUpdate = { 0, 0.0f, 0.0f, m_clock.now() };
		SetSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
	}

	Simulation::~Simulation() {
	}

	void Simulation::PreInitialize() {
		Logger::info(L"Game Name:\t%s\n", GameSettings::game_name());
		Logger::info(L"Boot Time:\t%s\n", Time::get_datetime().c_str());
		Logger::info(L"Engine Mode:\t%s\n", Engine::ModeToString().c_str());
		Logger::info(L"Current Path:\t%s\n", GameSettings::CurrentPath().c_str());
		
		// Load
		Logger::info(L"Registering Simulation window...\n");
		win32::Window::RegisterNewClass();
		win32::Window::Build();
		Logger::info(L"Registering Metrics window...\n");
		
		Logger::info(L"Initializing DX12 context...\n");
		dx12::Application::CreateApplication(L"MainApplication", DEFAULT_WIDTH, DEFAULT_HEIGHT, Handle());

		// End Load
		Logger::log_debug_seperator();
	}

	void Simulation::Update() { }

	void Simulation::Teardown() {
		dx12::Application::DestroyApplication();
	}


	LRESULT Simulation::MessageHandler(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
		Logger::debug(L"Simulation Handler: %d\n", message);
		if (dx12::Application::IsInitialized()) {
			switch (message) {
				case WM_PAINT:		{  }				break;
				case WM_SIZE:		{ Resize(); }			break;
				case WM_DESTROY:	{ PostQuitMessage(0); }	break;
			}
		}

		return Window::MessageHandler(hwnd, message, wParam, lParam);
	}


	void Simulation::Resize() {
		RECT client = {};
		GetClientRect(Handle(), &client);
		ResizeEvent event = {
			client.right - client.left,
			client.bottom - client.top
		};

		dx12::Application::Get()->Resize(event.newWidth, event.newHeight);
		OnResize(event);
	}
	
	void Simulation::Tick() {
		
		// Update
		auto nowTime = m_clock.now();
		auto delta = nowTime - m_currentUpdate.lastUpdate;

		m_currentUpdate.elapsedSeconds += (delta.count() * 1e-9);
		m_currentUpdate.frameCounter += 1;
		m_currentUpdate.lastUpdate = nowTime;

		FPSCounter::UpdateFrameCount(m_currentUpdate.frameCounter);
		if (m_currentUpdate.elapsedSeconds > 1.0f) {
			auto fps = m_currentUpdate.frameCounter / m_currentUpdate.elapsedSeconds;
			FPSCounter::UpdateFPS(fps);

			m_currentUpdate.totalTime += m_currentUpdate.elapsedSeconds;
			FPSCounter::UpdateTotalTime(m_currentUpdate.totalTime);

			m_currentUpdate.frameCounter = 0;
			m_currentUpdate.elapsedSeconds = 0.0f;
		}
		OnUpdate(m_currentUpdate);

		// Render
		OnRender({});
	}
}

