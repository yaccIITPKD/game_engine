#include "../engine.h"

#include <windows.h>

funcdef void *
os_reserve(u64 size)
{
	void *result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
	return result;
}

funcdef bool
os_commit(void *ptr, u64 size)
{
	void *result = VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
	return result != 0;
}

funcdef void
os_decommit(void *ptr, u64 size)
{
	VirtualFree(ptr, size, MEM_DECOMMIT);
}

funcdef void
os_release(void *ptr, u64 size)
{
	(void) size;
	VirtualFree(ptr, 0, MEM_RELEASE);
}
