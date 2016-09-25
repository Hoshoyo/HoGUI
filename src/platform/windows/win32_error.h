#pragma once

#define PLATFORM_FATAL_ERROR(x, y) MessageBox(0, x, y, 0)
#define PLATFORM_ERROR(x, y)

// FILES
#define PlatformGetFileSize(file_handle) GetFileSize(file_handle, 0)

// ALLOC
#define PlatformAlloc(size, type) (type*)VirtualAlloc(0, size, MEM_COMMIT, PAGE_READWRITE);