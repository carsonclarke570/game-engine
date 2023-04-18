#pragma once

#include "resources/ResourceManager.h"

class DAYBREAK_API GameSettings {
	
	private:
		static GameSettings* inst;
		static GameSettings* instance() { return inst; }

		/* Per game variables */
		WCHAR m_game_name[MAX_NAME_STRING];
		WCHAR m_game_short_name[MAX_NAME_STRING];
		WCHAR m_game_boot_time[MAX_NAME_STRING];
		HICON m_game_icon;
		WCHAR m_splashURL[MAX_NAME_STRING];

		std::wstring m_currentPath;

		/* DirectX12 Settings */
		bool m_useWARP;
		bool m_useVSync;
		
	public:
		GameSettings();
		~GameSettings();

		static WCHAR* game_name() { return inst->m_game_name; }
		static void set_game_name(int id) { ResourceManager::load_string(id, inst->m_game_name, MAX_NAME_STRING); }

		static WCHAR* game_short_name() { return inst->m_game_short_name; }
		static void set_game_short_name(int id) { ResourceManager::load_string(id, inst->m_game_short_name, MAX_NAME_STRING); }

		static HICON game_icon() { return inst->m_game_icon; }
		static void set_game_icon(int id) { inst->m_game_icon = ResourceManager::load_icon(id); }

		static WCHAR* game_boot_time() { return inst->m_game_boot_time; }

		static WCHAR* SplashURL() { return inst->m_splashURL; }
		static void SetSplashURL(int id) { ResourceManager::load_string(id, inst->m_splashURL, MAX_NAME_STRING); }

		static bool UseWARP() { return inst->m_useWARP; }
		static void SetUseWARP(bool useWARP) { inst->m_useWARP = useWARP; }

		static std::wstring CurrentPath() { return inst->m_currentPath; }
		static void SetCurrentPath(std::wstring path) { inst->m_currentPath = path; }
};