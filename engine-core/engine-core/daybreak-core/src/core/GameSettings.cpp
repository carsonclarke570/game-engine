#include "daybreak.h"

GameSettings* GameSettings::inst;

GameSettings::GameSettings() {
	inst = this;

	wcscpy_s(inst->m_game_name, L"undefined");
	wcscpy_s(inst->m_game_short_name, L"undefined");
	wcscpy_s(inst->m_game_boot_time, Time::get_datetime(true).c_str());
	wcscpy_s(inst->m_splashURL, L"..\\daybreak-core\\res\\images\\daybreak.bmp");

	inst->m_currentPath = std::wstring(std::filesystem::current_path().c_str());
	inst->m_useWARP = false;
}

GameSettings::~GameSettings() {
}
