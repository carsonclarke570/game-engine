#include "daybreak.h"
#include "ResourceManager.h"

HICON ResourceManager::load_icon(int icon_id) {
	return LoadIcon(HInstance(), MAKEINTRESOURCE(icon_id));
}

void ResourceManager::load_string(int string_id, LPWSTR dest, int max) {
	LoadString(HInstance(), string_id, dest, max);
}