#pragma once

#include <string>

namespace Time {
	std::wstring DAYBREAK_API get_time(bool stripped = false);
	std::wstring DAYBREAK_API get_date(bool stripped = false);
	std::wstring DAYBREAK_API get_datetime(bool stripped = false);
}