#pragma once

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>

#include "windows/win32_error.h"
#else
#error Platform not supported
#endif