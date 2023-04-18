#pragma once

#include <string>

class DAYBREAK_API Logger {
	private:
		static Logger* inst;
		static Logger* instance() { return inst; }

		static std::wstring log_dir();
		static std::wstring log_file();

		static void log(const wchar_t* level, const wchar_t* fmt, va_list args);
	
	public:
		Logger();
		~Logger();

		static void info(const wchar_t* fmt, ...);
		static void debug(const wchar_t* fmt, ...);
		static void error(const wchar_t* fmt, ...);

		static void log_debug_seperator();
		static bool is_mtail_running();
		static void StartMTail();
};