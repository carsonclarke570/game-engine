#include "daybreak.h"

#include "InputManager.h"

DX12InputManager::DX12InputManager() : current_msg{ 0 } {}

bool DX12InputManager::handle_exit() {
	return current_msg.message == WM_QUIT;
}

void DX12InputManager::handle_input() {
	// If there are Window messages then process them.
	if (PeekMessage(&current_msg, 0, 0, 0, PM_REMOVE)) {
		TranslateMessage(&current_msg);
		DispatchMessage(&current_msg);
	}
}

InputManager& InputManagerFactory::create() {
	static DX12InputManager input_manager;
	return input_manager;
}