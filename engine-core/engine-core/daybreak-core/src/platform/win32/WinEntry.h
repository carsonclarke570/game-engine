#include "daybreak.h"

#include "engine/manager/WindowManager.h"
#include "engine/manager/RenderStateManager.h"
#include "input/InputManager.h"
#include "common/CmdLineArgs.h"

extern win32::IApplication* entry_point();

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ INT nCmdShow) {

	// Apply GAME settings to DAYBREAK
	GameSettings settings;
	WindowManagerUtil::Build(entry_point());

	// Read arguments and initialize logger
	CmdLine::ReadArguments();
	Logger logger;

	// Initialize
	Daybreak::RenderStateManager::Create();
	WindowManagerUtil::Initialize();
	WindowManagerUtil::Open();

	InputManager& input_manager = InputManagerFactory::create();
	while (!input_manager.handle_exit()) {
		bool hasInput = input_manager.handle_input();
		if (!hasInput) {
			WindowManagerUtil::Tick();
		}
	}

	WindowManagerUtil::Close();
	Daybreak::RenderStateManager::Destroy();

	if (Engine::GetMode() == EngineMode::DEBUG || Engine::GetMode() == EngineMode::EDITOR) {
		Microsoft::WRL::ComPtr<IDXGIDebug1> dxgi_debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgi_debug.GetAddressOf())))) {
			dxgi_debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
		}
	}

	return 0;
}