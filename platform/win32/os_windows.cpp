#include "../engine.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fileapi.h>

funcdef void *
os_reserve(u64 size)
{
    return VirtualAlloc(0, size, MEM_RESERVE, PAGE_NOACCESS);
}

funcdef bool
os_commit(void *ptr, u64 size)
{
    return VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != 0;
}

funcdef void
os_decommit(void *ptr, u64 size)
{
    VirtualFree(ptr, size, MEM_DECOMMIT);
}

funcdef void
os_release(void *ptr, u64 size)
{
    (void)size;
    VirtualFree(ptr, 0, MEM_RELEASE);
}

funcdef OS_TimeStamp
os_time_now()
{
    static LARGE_INTEGER frequency = {};
    static bool initialized = false;

    if (!initialized)
    {
        QueryPerformanceFrequency(&frequency);
        initialized = true;
    }

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    return (u64)((counter.QuadPart * 1000000000ULL) /
                 frequency.QuadPart);
}

funcdef OS_FileData
os_file_data(string path)
{
    OS_FileData result = {};

    Temp t = temp_begin(scratch(0, 0));
    defer(temp_end(t));

    string cstr = string_to_cstring(t.arena, path);

    WIN32_FILE_ATTRIBUTE_DATA data = {};
    if (!GetFileAttributesExA(
            (char *)cstr.raw,
            GetFileExInfoStandard,
            &data))
    {
        return result;
    }

    result.flags |= File_Exists;

    if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        result.flags |= File_Directory;

    ULARGE_INTEGER size = {};
    size.HighPart = data.nFileSizeHigh;
    size.LowPart  = data.nFileSizeLow;
    result.size = size.QuadPart;

    ULARGE_INTEGER timestamp = {};
    timestamp.HighPart = data.ftLastWriteTime.dwHighDateTime;
    timestamp.LowPart  = data.ftLastWriteTime.dwLowDateTime;
    result.timestamp = timestamp.QuadPart;

    char const *ext = nullptr;
    for (char const *c = (char *)cstr.raw; *c; ++c)
    {
        if (*c == '.')
            ext = c;
    }

    if (ext)
    {
        if (_stricmp(ext, ".png")  == 0 ||
            _stricmp(ext, ".jpg")  == 0 ||
            _stricmp(ext, ".jpeg") == 0)
        {
            result.kind = OS_FileKind::Image;
        }
        else if (_stricmp(ext, ".glsl") == 0 ||
                 _stricmp(ext, ".vert") == 0 ||
                 _stricmp(ext, ".frag") == 0)
        {
            result.kind = OS_FileKind::Shader;
        }
        else if (_stricmp(ext, ".data") == 0)
        {
            result.kind = OS_FileKind::Data;
        }
        else if (_stricmp(ext, ".ttf") == 0)
        {
            result.kind = OS_FileKind::Font;
        }
        else if (_stricmp(ext, ".dll") == 0)
        {
            result.kind = OS_FileKind::Dynamic_Library;
        }

        if (_stricmp(ext, ".exe") == 0 ||
            _stricmp(ext, ".bat") == 0 ||
            _stricmp(ext, ".cmd") == 0 ||
            _stricmp(ext, ".com") == 0)
        {
            result.flags |= File_Executable;
        }
    }

    return result;
}

funcdef Load_Error
os_file_to_buffer(u8 *ptr, u64 len, string path)
{
    Temp t = temp_begin(scratch(0, 0));
    defer(temp_end(t));

    string cstr = string_to_cstring(t.arena, path);

    HANDLE file = CreateFileA(
        (char *)cstr.raw,
        GENERIC_READ,
        FILE_SHARE_READ,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0);

    if (file == INVALID_HANDLE_VALUE)
        return Load_IO_Error;

    defer(CloseHandle(file));

    LARGE_INTEGER size;

    if (!GetFileSizeEx(file, &size))
        return Load_IO_Error;

    if (!ptr || len == 0)
        return Load_Buffer_Overflow;

    u64 file_size = (u64)size.QuadPart;
    u64 load_size = Min(file_size, len);

    u64 total = 0;

    while (total < load_size)
    {
        DWORD chunk =
            (DWORD)Min(load_size - total,
                       (u64)0x7fffffff);

        DWORD read = 0;

        if (!ReadFile(
                file,
                ptr + total,
                chunk,
                &read,
                0))
        {
            return Load_IO_Error;
        }

        if (read == 0)
            return Load_IO_Error;

        total += read;
    }

    return (len >= file_size)
               ? Load_Ok
               : Load_Buffer_Overflow;
}

funcdef bool
os_write_to_file(string path, bytes data)
{
    Temp t = temp_begin(scratch(0, 0));
    defer(temp_end(t));

    string cstr = string_to_cstring(t.arena, path);

    HANDLE file = CreateFileA(
        (char *)cstr.raw,
        GENERIC_WRITE,
        0,
        0,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        0);

    if (file == INVALID_HANDLE_VALUE)
        return false;

    defer(CloseHandle(file));

    u64 total = 0;

    while (total < data.len)
    {
        DWORD chunk =
            (DWORD)Min(data.len - total,
                       (u64)0x7fffffff);

        DWORD written = 0;

        if (!WriteFile(
                file,
                data.raw + total,
                chunk,
                &written,
                0))
        {
            return false;
        }

        if (written == 0)
            return false;

        total += written;
    }

    return true;
}

funcdef OS_Handle
os_load_library(string path)
{
    Temp t = temp_begin(scratch(0, 0));
    defer(temp_end(t));

    char *path_cstr = (char *)string_to_cstring(t.arena, path).raw;

    HMODULE handle = LoadLibraryA(path_cstr);
    if (!handle) {
        printf("LoadLibraryA failed (%lu)\n", GetLastError());
    }
    assert(handle);

    return OS_Handle{
        (uintptr_t)handle
    };
}

funcdef void *
os_load_symbol(OS_Handle lib, const char *name)
{
    HMODULE handle = (HMODULE)lib.v;
    return (void *)GetProcAddress(handle, name);
}

funcdef void
os_unload_library(OS_Handle lib)
{
    if (lib.v) {
        FreeLibrary((HMODULE)lib.v);
    }
}

funcdef void
os_time_sleep(OS_TimeDuration duration)
{
    u64 ns = os_duration_to_ns(duration);

    DWORD ms = (DWORD)(ns / 1000000ull);

    if (ms == 0 && ns > 0) {
        ms = 1;
    }

    Sleep(ms);
}
