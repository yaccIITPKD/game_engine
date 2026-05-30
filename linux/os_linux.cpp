#include "../engine.h"

#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>

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


funcdef OS_FileData
os_file_data(string path)
{
	OS_FileData result = {};
	
	Temp t = {};
	defer(temp_end(t));

	string cstr = string_to_cstring(scratch(&t), path);

	struct stat st;
	if (stat((char *)cstr.raw, &st) != 0) {
		return result;
	}

	result.flags |= File_Exists;

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
		}
		else if (strcasecmp(ext, ".ttf")) {
			result.kind = OS_FileKind::Font;
		}
		else if (strcasecmp(ext, ".so") == 0) {
			result.kind = OS_FileKind::Dynamic_Library;
		}
	}

	return result;
}
