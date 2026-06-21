#include "../../engine.h"

#include <errno.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <libgen.h>

funcdef void *
os_reserve(u64 size)
{
	void *result = mmap(0, size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
	if(result == MAP_FAILED)
	{
		result = 0;
	}
	return result;
}

funcdef bool
os_commit(void *ptr, u64 size)
{
	mprotect(ptr, size, PROT_READ|PROT_WRITE);
	return 1;
}

funcdef void
os_decommit(void *ptr, u64 size)
{
	madvise(ptr, size, MADV_DONTNEED);
	mprotect(ptr, size, PROT_NONE);
}

funcdef void
os_release(void *ptr, u64 size)
{
	munmap(ptr, size);
}



funcdef OS_TimeStamp
os_time_now() 
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	return (u64)ts.tv_sec * 1000000000ULL + (u64)ts.tv_nsec;
}

funcdef void
os_time_sleep(OS_TimeDuration duration)
{
	u64 ns = os_duration_to_ns(duration);

	struct timespec ts;
	ts.tv_sec  = ns / 1000000000ull;
	ts.tv_nsec = ns % 1000000000ull;

	while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {
		// continue sleeping for remaining time
	}
}

funcdef OS_FileData
os_file_data(string path)
{
	OS_FileData result = {};
	
	Temp t = temp_begin(scratch(0, 0));
	defer(temp_end(t));

	string cstr = string_to_cstring(t.arena, path);

	struct stat st;
	if (stat((char *)cstr.raw, &st) != 0) {
		return result;
	}

	result.flags |= File_Exists;
	result.timestamp = (u64)st.st_mtim.tv_sec * 1000000000ULL + (u64)st.st_mtim.tv_nsec;

	if (S_ISDIR(st.st_mode)) {
		result.flags |= File_Directory;
	}

	if (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
		result.flags |= File_Executable;
	}

	result.size = (u64)st.st_size;

	char const *ext = nullptr;
	for (char const *c = (char *)cstr.raw; *c; ++c) {
		if (*c == '.') {
			ext = c;
		}
	}

	if (ext) {
		if (strcasecmp(ext, ".png")  == 0 ||
			strcasecmp(ext, ".jpg")  == 0 ||
			strcasecmp(ext, ".jpeg") == 0
		) {
			result.kind = OS_FileKind::Image;
		}
		else if (strcasecmp(ext, ".glsl") == 0 ||
				 strcasecmp(ext, ".vert") == 0 ||
				 strcasecmp(ext, ".frag") == 0
		) {
			result.kind = OS_FileKind::Shader;
		} else if (strcasecmp(ext, ".data") == 0) {
			result.kind = OS_FileKind::Data;
		}
		else if (strcasecmp(ext, ".ttf") == 0) {
			result.kind = OS_FileKind::Font;
		}
		else if (strcasecmp(ext, ".so") == 0) {
			result.kind = OS_FileKind::Dynamic_Library;
		}
	}

	return result;
}

funcdef Load_Error
os_file_to_buffer(u8 *ptr, u64 len, string path)
{
	Temp t = temp_begin(scratch(0, 0));
	defer(temp_end(t));

	string cstring = string_to_cstring(t.arena, path);
	
	int fd = open((char *)cstring.raw, O_RDONLY);
	if (fd < 0) {
		return Load_IO_Error;
	}
	defer(close(fd));

	struct stat st;
	if (fstat(fd, &st) < 0) 
		return Load_IO_Error;
	
	if (!S_ISREG(st.st_mode)) 
		return Load_IO_Error;

	u64 size = (u64)st.st_size;

	if (!ptr || len == 0) 
		return Load_Buffer_Overflow;

	u64 load_size = Min(size, len);
	u64 total = 0;

	while (total < load_size) {
		ssize_t n = read(fd, ptr + total, (size_t)(load_size - total));

		if (n < 0) {
			if (errno == EINTR) {
				continue;
			}
			return Load_IO_Error;
		}

		if (n == 0) { return Load_IO_Error; }
		total += (u64)n;
	}

	return (len >= size) ? Load_Ok : Load_Buffer_Overflow;
}

funcdef bool
os_write_to_file(string path, bytes data)
{
	Temp t = temp_begin(scratch(0, 0));
	defer(temp_end(t));


	string cstring = string_to_cstring(t.arena, path);
	if (!cstring.raw)
		return false;

	mode_t mode = 0644;

	struct stat st;
	if (stat((char *)cstring.raw, &st) == 0)
		mode = st.st_mode & 0777;

	int fd = open((char *)cstring.raw, O_WRONLY | O_CREAT | O_TRUNC, mode);

	if (fd < 0)
		return false;

	defer(close(fd));

	u64 total = 0;
	while (total < data.len) {
		ssize_t n = write(fd, data.raw + total, (size_t)(data.len - total));

		if (n < 0) {
			if (errno == EINTR)
				continue;

			return false;
		}

		if (n == 0)
			return false;

		total += (u64)n;
	}

	return true;
}


funcdef OS_Handle
os_load_library(string path)
{
	Temp t = temp_begin(scratch(0, 0));
	defer(temp_end(t));

	void *handle = dlopen((char *) string_to_cstring(t.arena, path).raw, RTLD_NOW);
	if (!handle) {
		printf("%s\n", dlerror());
	}
	assert(handle);

	return OS_Handle {
		(uintptr_t) handle
	};
}

funcdef void *
os_load_symbol(OS_Handle lib, const char *name)
{
	void *handle = (void *) lib.v;
	void *sym_ptr = dlsym(handle, name);
	return sym_ptr;
}

funcdef void
os_unload_library(OS_Handle lib)
{
	if (lib.v)
		dlclose((void *) lib.v);
}

funcdef void
os_set_working_dir(string dir)
{
	if (!dir.raw || !dir.len) return;

	local_persist char path[4098];
	u64 len = dir.len;

	if (len >= sizeof(path)) {
		len = sizeof(path) - 1;
	}

	memcpy(path, dir.raw, len);
	path[len] = '\0';

	int result = chdir(path);
}

funcdef string
os_get_working_dir(Arena *arena)
{
	Temp t0 = temp_begin(scratch(&arena, 1));
	defer(temp_end(t0));

	slice<char> temp_page = alloc_slice(t0.arena, char, KB(4));
	char *cwd = getcwd(temp_page.raw, temp_page.len);

	string str = { (u8 *) cwd, strlen(cwd) };
	return string_copy(arena, str);
}


funcdef string
os_get_exec_directory(Arena *arena)
{
	Temp t0 = temp_begin(scratch(&arena, 1));
	defer(temp_end(t0));

	slice<char> temp_page = alloc_slice(t0.arena, char, KB(4));
	int count = readlink("/proc/self/exe", temp_page.raw, temp_page.len - 1);
	temp_page[count] = '\0';

	char *dir = dirname(temp_page.raw);

	string str = { (u8 *) dir, (u64) strlen(dir) };
	return string_copy(arena, str);
}
