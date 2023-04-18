#pragma once

#ifdef WIN32
	#include "d3dx12.h"
	#include <Windows.h>
	#include <wrl.h>
	#include <d3d12.h>
	#include <dxgi1_6.h>
	#include <d3dcompiler.h>
	#include <dxgidebug.h>
	#include <DirectXMath.h>
	#include <DirectXTex.h>

	using namespace DirectX;
	using namespace Microsoft::WRL;
	#pragma comment(lib, "d3d12.lib")
	#pragma comment(lib, "dxgi.lib")
	#pragma comment(lib, "dxguid.lib")
	#pragma comment(lib, "d3dcompiler.lib")

#endif

#include <string>
#include <list>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <queue>
#include <memory>
#include <filesystem>
#include <set>
#include <map>
#include <functional>
#include <locale>
#include <codecvt>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#pragma comment(lib, "assimp-vc143-mtd.lib")

#include "core/Core.h"