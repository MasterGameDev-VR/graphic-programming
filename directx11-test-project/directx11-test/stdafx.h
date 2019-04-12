// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define _CRT_SECURE_NO_WARNINGS // stop unsafe warning for not using CRT version of functions
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define NOMINMAX				// remove Windows min and max macros

// Windows Header Files:
#include <windows.h>
#include <windowsx.h>

// STL headers
#include <algorithm>
#include <memory>
#include <vector>
#include <array>
#include <map>

// DirectX headers
#include <d3d11_1.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <wrl/client.h>

// Project headers
#include <common/assert.h>
#include <common/common_macros.h>
#include <common/int_types.h>
#include <render/render_macros.h>

// DirectXTK precompile header
#include "external_libs/directxtk/pch.h"