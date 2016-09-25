// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define NOMINMAX
#define _USE_MATH_DEFINES
#include <windows.h>

#include <algorithm>
#include <vector>
#include <cmath>
#include <chrono>
#include <map>

#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

#include "af_math.h"
#include "helper.h"
#include "helper_win.h"
#include "helper_text.h"
#include "helper_vulkan.h"
#include "matrix_man.h"
#include "dib.h"
#include "fps.h"
#include "font_man.h"
#include "dev_camera.h"
#include "system_misc.h"
#include "triangle.h"
#include "sky.h"
#include "app.h"
