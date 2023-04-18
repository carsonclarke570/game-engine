#pragma once

namespace WindowManagerUtil {

	void DAYBREAK_API Build(win32::IApplication* app);
	void DAYBREAK_API Initialize();
	void DAYBREAK_API Tick();
	void DAYBREAK_API Open();
	void DAYBREAK_API Close();
}

class DAYBREAK_API WindowManager {

	private:
		std::unordered_map<std::wstring, win32::Window*> m_windows;

	public:
		WindowManager();
		~WindowManager();

		void Add(std::wstring windowName, win32::Window* window);
		win32::Window* Get(std::wstring windowName);
		void Initialize(std::wstring windowName);
		void InitializeAll();
		void Open(std::wstring windowName);
		void OpenAll();
};