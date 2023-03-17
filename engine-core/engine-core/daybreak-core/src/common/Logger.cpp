#include "daybreak.h"

#include <fstream>
#include <Shlobj.h>
#include <cstdio>
#include <tlhelp32.h>

Logger* Logger::inst;

Logger::Logger() {
	inst = this;
}

Logger::~Logger() {}

void Logger::log(const wchar_t* level, const wchar_t* fmt, va_list args) {
	wchar_t buffer[4096];
	vswprintf_s(buffer, fmt, args);
	OutputDebugString(buffer);

	std::wfstream outfile;
	outfile.open(std::wstring(log_dir() + L"/" + log_file()), std::ios_base::app);

	if (outfile.is_open()) {
		std::wstring s = buffer;
		outfile << L"[" << Time::get_datetime() << L" - " << level << "]  " << s;
		outfile.close();
		OutputDebugString(s.c_str());
	} else {
		MessageBox(NULL, L"Unable to open log file...", L"Log Error", MB_OK);
	}
}

void Logger::info(const wchar_t* fmt, ...) {
	va_list args;

	va_start(args, fmt);
	log(L"INFO", fmt, args);
	va_end(args);
}

void Logger::debug(const wchar_t* fmt, ...) {
	va_list args;

	va_start(args, fmt);
	log(L"DEBG", fmt, args);
	va_end(args);
}

void Logger::error(const wchar_t* fmt, ...) {
	va_list args;

	va_start(args, fmt);
	log(L"ERROR", fmt, args);
	va_end(args);
}

std::wstring Logger::log_dir() {
	wchar_t path[1024];
	wchar_t* app_data_local;

	SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &app_data_local);
	wcscpy_s(path, app_data_local);
	wcscat_s(path, L"\\");
	wcscat_s(path, GameSettings::game_name());

	CreateDirectory(path, NULL);
	wcscat_s(path, L"\\Log");
	CreateDirectory(path, NULL);
	return path;
}

std::wstring Logger::log_file() {
	wchar_t file[1024];
	wcscpy_s(file, GameSettings::game_name());
	wcscat_s(file, GameSettings::game_boot_time());
	wcscat_s(file, L".log");
	return file;
}

void Logger::log_debug_seperator()
{
	std::wstring s = L"\n------------------------------------------------------------------------------------\n\n";

#ifdef _DEBUG
	std::wfstream outfile;
	outfile.open(std::wstring(log_dir() + L"/" + log_file()), std::ios_base::app);

	if (outfile.is_open()) {
		outfile << s;
		outfile.close();
	} else {
		MessageBox(NULL, L"Unable to open log file...", L"Log Error", MB_OK);
	}
#endif
}

bool Logger::is_mtail_running() {
	bool exists = false;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry))
		while (Process32Next(snapshot, &entry))
			if (!_wcsicmp(entry.szExeFile, L"mTAIL.exe"))
				exists = true;

	CloseHandle(snapshot);
	return exists;
}

void Logger::StartMTail() {
	if (is_mtail_running()) {
		Logger::error(L"--MTail failed to start - Already Running\n");
		return;
	}

	Logger::info(L"--Starting MTail\n");
	WCHAR path[MAX_PATH] = { 0 };
	GetCurrentDirectoryW(MAX_PATH, path);
	std::wstring url = path + std::wstring(L"/mTAIL.exe");
	std::wstring params = L" \"" + log_dir() + L"/" + log_file() + L"\" /start";
	ShellExecute(0, NULL, url.c_str(), params.c_str(), NULL, SW_SHOWDEFAULT);
}