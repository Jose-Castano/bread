#pragma once
#include "Core/Header.hpp"


#if defined(BREAD_ANDROID)
#include "Platform/android/android_header.h"
#elif defined(BREAD_WIN32)
#include "Platform/Win32/win32_header.h"
#endif
