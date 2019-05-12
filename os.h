#pragma once
#include "common.h"

char*       os_file_read(const char* path, s32* file_length, void*(*allocator)(size_t));
void        os_file_free(char* buf);
s32         os_file_write(const char* path, const char* buf, s32 size);
r64         os_time_us();
void        os_usleep(u64 microseconds);
const char* os_relative_from_fullpath(const char* fullpath);
u64         os_file_last_modified(const char* filename);
bool        os_copy_file(const char* filename, const char* copy_name);
#if defined(_WIN32) || defined(_WIN64)
char*       realpath(const char* file_name, char* resolved_name);
#endif