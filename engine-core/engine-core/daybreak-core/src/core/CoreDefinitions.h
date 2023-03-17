#pragma once

#ifdef BUILD_DLL
	#define DAYBREAK_API __declspec(dllexport)
#else
	#define DAYBREAK_API __declspec(dllimport)
#endif

#define MAX_NAME_STRING 256
#define HInstance() GetModuleHandle(NULL)