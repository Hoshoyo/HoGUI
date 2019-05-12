#include "os.h"
#include <stdio.h>
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>

char* realpath(const char* file_name, char* resolved_name) {
	DWORD status = GetFullPathNameA(file_name, 2048, resolved_name, 0);
	if (status > 2048) {
		printf("Could not retrieve full path of file %s: buffer is too small", file_name);
		return 0;
	}
	for (int i = 0; resolved_name[i]; ++i) {
		if (resolved_name[i] == '\\')
			resolved_name[i] = '/';
	}
	return resolved_name;
}

#elif defined(__linux__)
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#endif

char* os_file_read(const char* path, s32* file_length, void*(*allocator)(size_t))
{
	FILE* file;
	s8* buffer;
	s32 len;

	file = fopen(path, "rb");
    if (file == NULL)
    {
        printf("os_file_read: could not open file %s", path);
        return NULL;
    }

	fseek(file, 0, SEEK_END);
	len = ftell(file);
	rewind(file);

	buffer = (s8*)allocator((len + 1) * sizeof(char));
	if (fread(buffer, 1, len, file) != len)
    {
        printf("os_file_read: could not read file %s", path);
        fclose(file);
        free(buffer);
        return NULL;
    }

	fclose(file);

	buffer[len] = '\0';

	if (file_length)
		*file_length = len;

	return buffer;
}

void os_file_free(char* buf)
{
	free(buf);
}

s32 os_file_write_new(const char* path, const char* buf, s32 size)
{
	FILE *fptr;
	fptr = fopen(path, "wb+");
	if (fptr == NULL)
	{
		printf("os_file_write: could not open file %s", path);
		return -1;
	}

	if (fwrite(buf, 1, size, fptr) != size)
	{
		printf("os_file_write: could not write to file %s", path);
		return -1;
	}

	fclose(fptr);

	return 0;
}

s32 os_file_write(const char* path, const char* buf, s32 size)
{
   FILE *fptr;
   fptr = fopen(path, "wb");
   if (fptr == NULL)
   {
        printf("os_file_write: could not open file %s", path);
        return -1;
   }

   if (fwrite(buf, 1, size, fptr) != size)
   {
        printf("os_file_write: could not write to file %s", path);
        return -1;
   }

   fclose(fptr);

   return 0;
}

#if defined(_WIN32) || defined(_WIN64)
static r64 perf_frequency;
static void os_set_query_frequency() {
	LARGE_INTEGER li = { 0 };
	QueryPerformanceFrequency(&li);
	perf_frequency = (r64)(li.QuadPart);
}

r64 os_time_us() {
	static initialized = false;
	if (!initialized) {
		os_set_query_frequency();
		initialized = true;
	}

	LARGE_INTEGER li = { 0 };
	QueryPerformanceCounter(&li);
	return ((r64)(li.QuadPart) / perf_frequency) * 1000000.0;
}
void os_usleep(u64 microseconds) {
	// TODO(psv): figure this out
}

u64
os_file_last_modified(const char* filename) {
	WIN32_FIND_DATA file_info;
	FindFirstFileA(filename, &file_info);
	FILETIME last_written = file_info.ftLastWriteTime;

	return *(u64*)&last_written;
}
#elif defined(__linux__)

r64 os_time_us()
{
	clockid_t clockid;
	struct timespec t_spec;
	int start = clock_gettime(CLOCK_MONOTONIC_RAW, &t_spec);
	u64 res = t_spec.tv_nsec + 1000000000 * t_spec.tv_sec;
	return (r64)res / 1000.0;
}

void os_usleep(u64 microseconds)
{
	usleep(microseconds);
}

u64
os_file_last_modified(const char* filename) {
	struct stat st;
	if (stat(filename, &st))
		printf("os_file_last_modified: %s", strerror(errno));
	time_t mtime = st.st_mtime;

	return (u64)mtime;
}

#else
#endif

const char* 
os_relative_from_fullpath(const char* fullpath) {
	char buffer[4096] = { 0 };
	char* pwd = realpath(".", buffer);

	u64 i = 0;
	for (; pwd[i] == fullpath[i]; ++i);

	if (fullpath[i] == '/' || fullpath[i] == '\\') {
		i++;
	}

	return fullpath + i;
}

bool
os_copy_file(const char* filename, const char* copy_name) {
	s32 length;
	char* data = os_file_read(filename, &length, malloc);
	if (!data) return false;

	if (os_file_write_new(copy_name, data, length) == -1)
		return false;
	return true;
}
