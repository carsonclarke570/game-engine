#pragma once

#include "CoreDefinitions.h"

#include "engine/Engine.h"

#include "common/Logger.h"
#include "common/Time.h"

#include "core/GameSettings.h"

#ifdef WIN32
	#include "platform/win32//Win32Utils.h"
	#include "platform/win32/SubObject.h"
	#include "platform/win32/Win32Caption.h"
	#include "platform/win32/Window.h"
	#include "platform/win32/IApplication.h"
#endif