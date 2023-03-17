#pragma once

// TODO: Abstract this so it's not Windows specific
class DAYBREAK_API ResourceManager {
	public:
		static HICON load_icon(int icon_id);
		static void load_string(int string_id, LPWSTR dest, int max);
};