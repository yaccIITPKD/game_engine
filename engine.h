#ifndef ENGINE_H
#define ENGINE_H

//////////
// ~gaureesh @NOTE: base types

#include <stdint.h> // for fixed size integers.
#include <assert.h> // for assert macro.
#include <string.h> // for memmove, memcpy etc. ( not for cstd string functions ).
#include <stdarg.h> // for va_args.
#include <stdio.h>  // standard io operations.

typedef uint8_t  u8;
typedef  int8_t  s8;
typedef uint16_t u16;
typedef  int16_t s16;
typedef uint32_t u32;
typedef  int32_t s32;
typedef uint64_t u64;
typedef  int64_t s64;

typedef float  f32;
typedef double f64;

template<typename T>
struct slice {
	T   *raw;
	u64  len;
	
	slice<T> range(u64 begin, u64 end) {
		assert(begin <= len);
		assert(begin <= end);
		assert(end <= len);
		
		return {
			raw + begin,
			end - begin
		};
	}

	T& operator[](u64 index) {
		assert(index < this->len);
		return this->raw[index];
	}
};

template<typename T>
struct list {
	T   *raw;
	u64  len;
	u64  capacity;

	T& operator[](u64 index) {
		assert(index < this->len);
		return this->raw[index];
	}
};

template<typename T> list<T> list_make(slice<T> buf);

template<typename T> void append(list<T> *l, T value);
template<typename T> void append_slice(list<T> *l, slice<T> values);
template<typename T> void insert_slice(list<T> *l, u64 index, slice<T> values);
template<typename T> void clear(list<T> *l);

struct vec2 {
	f32 x, y;
};

struct vec3 {
	f32 x, y, z;
};

struct vec4 {
	f32 x, y, z, w;
};


typedef slice<u8> bytes;
typedef slice<const u8> string;

#define S(x) string { (const u8 *) (x), sizeof(x) - 1 }
#define S_FMT(s) (int) s.len, (char *) s.raw

//////////
// ~gaureesh @NOTE: os and compiler detection

#ifdef _WIN32
# define OS_Windows 1
# define OS_Linux   0
# define OS_Mac     0
#elif __linux__
# define OS_Windows 0
# define OS_Linux   1
# define OS_Mac     0
#elif __APPLE__
# define OS_Windows 0
# define OS_Linux   0
# define OS_Mac     1
#else
# error "target platform unsupported"
#endif

#if defined(__clang__)
# define Compiler_Clang
#elif defined(__GNUC__) || defined(__GNUG__)
# define Compiler_GCC
#elif defined(_MSC_VER)
# define Compiler_CL
#else
# error "c++ compiler not supported"
#endif

#ifndef __cplusplus
# error "compiler should be c++"
#endif


//////////////
// ~gaureesh @NOTE: useful macro definitions

#ifndef DEBUG_BUILD
# define DEBUG_BUILD 1
#endif

#define funcdef       static
#define local_persist static
#define global        static

#define Max(a, b) ((a) > (b) ? (a) : (b))
#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Clamp(min, val, max)  Min(Max((val), (min)), (max))

#define Align_Up_Power_2(val, alignment) (((val) + (alignment) - 1) & ~((alignment) - 1));

#define KB(x) (u64) (x << 10)
#define MB(x) (u64) (x << 20)
#define GB(x) (u64) (x << 30)

#define Flag_Check(__flags, __mask) !!((__flags) & (__mask))

funcdef inline u32
byteswap_u32(u32 x)
{
    return ((x & 0x000000FFu) << 24) |
           ((x & 0x0000FF00u) <<  8) |
           ((x & 0x00FF0000u) >>  8) |
           ((x & 0xFF000000u) >> 24);
}

struct Arena {
	u64   reserved;
	u64   committed;
	u64   used;
};

//////////////
// ~gaureesh @NOTE: os

struct OS_Handle { uintptr_t v; };

struct OS_Input {
};

enum class OS : s32 {
	Windows,
	Linux,
	Mac,
	Count,

#if OS_Windows
	Current = OS::Windows,
#elif OS_Linux
	Current = OS::Linux,
#elif OS_MAC
	Current = OS::Mac,
#endif
};

const string OS_STRINGS[(u32) OS::Count] = {
	S("Windows"),
	S("Linux"),
	S("Mac"),
};

funcdef void os_init();
funcdef void os_deinit();

funcdef void *os_reserve(u64 size);
funcdef bool  os_commit(void *ptr, u64 size);
funcdef void  os_decommit(void *ptr, u64 size);
funcdef void  os_release(void *ptr, u64 size);

funcdef OS_Handle os_open_window(string title);
funcdef void os_close_window(OS_Handle window);
funcdef bool os_window_should_close(OS_Handle window);
funcdef OS_Input os_prepare_frame(OS_Handle window);
funcdef vec2 os_window_size(OS_Handle window);

funcdef string os_string(OS os);
funcdef void *os_get_gl_proc_address();

typedef u64 OS_TimeStamp; // in nanoseconds
struct OS_TimeDuration {
	f64 seconds;
	f64 milliseconds;
	f64 microseconds;
};

funcdef OS_TimeStamp os_time_now();
funcdef OS_TimeDuration os_time_diff(OS_TimeStamp t0, OS_TimeStamp t1);

typedef u32 OS_FileFlags;
enum OS_FileFlag {
	File_Directory  = 1 << 0,
	File_Executable = 1 << 1,
	File_Exists     = 1 << 2,
};

enum class OS_FileKind : u32 {
	Unknown,

	Image,
	Dynamic_Library,
	Shader,
	Font,

	Data, // packed data for release builds
};

struct OS_FileData {
	OS_FileFlags flags;
	OS_FileKind kind;
	u64 size;
	u64 timestamp;
};

enum Load_Error {
	Load_Ok,
	Load_Not_Found,
	Load_Access_Denied,
	Load_Invalid_Path,
	Load_Buffer_Overflow,
	Load_IO_Error,
	Load_Error_Count,
};

funcdef OS_FileData os_file_data(string path);
funcdef Load_Error  os_file_to_buffer(u8 *ptr, u64 len, string path);
funcdef bytes       os_load_entire_file(Arena *arena, string path);
funcdef bool        os_write_to_file(string path, bytes data);

//////////////
// ~gaureesh @NOTE: arena

struct Temp {
	Arena *arena;
	u64    mark;
};

funcdef Arena *arena_make(u64 reserve);
funcdef void   arena_delete(Arena *arena);
funcdef void  *arena_allocate(Arena *arena, void *old_ptr, u64 old_size, u64 new_size, u64 alignment);
funcdef void   arena_free(Arena *arena);

funcdef Temp   temp_begin(Arena *arena);
funcdef void   temp_end(Temp temp);

funcdef Arena *scratch(Temp *temp = nullptr);

#define alloc_struct(_arr, _T) (_T *) arena_allocate((_arr), nullptr, 0, sizeof(_T), alignof(_T))
#define alloc_slice(_arr, _T, _n) slice<_T> { (_T *) arena_allocate((_arr), nullptr, 0, sizeof(_T) * (_n), alignof(_T)), (u64) (_n) }
#define realloc_slice(_arr, _s, _n) { (decltype((_s).data)) arena_allocate((_arr), (_s).data, sizeof(*(_s).data) * (_s).count, sizeof(*(_s).data) * (_n), alignof(decltype(*(_s).data))), (u64)(_n) }

//////////////
// ~gaureesh @NOTE: string

funcdef string string_from_bytes(bytes data);
funcdef string string_concat(Arena *arena, string a, string b);
funcdef string string_format(Arena *arena, const char *fmt_string, ...);
funcdef string string_from_cstring(Arena *arena, char *cstring);
funcdef slice<string> strings_from_cstrings(Arena *arena, int count, char **cstrings);

funcdef string string_to_cstring(Arena *arena, string s);

//////////////
// ~gaureesh @NOTE: graphics

typedef u32 color8; // in 0xFFBBGGAA format
#define Hex(x) byteswap_u32((u32)(x))

enum class Draw {
	Triangles, Lines
};

struct Vertex {
	vec3 position;
	vec2 texcoords;
	color8 color;
};

struct Camera {
	vec2 position;
	vec2 offset; // in pixels
	f32  scale;
};

struct Draw_Data {
	Draw primitive;
	Camera camera;

	list<Vertex> vertices;
	list<u16>    indices;

	// gl specific data
	u32 vao, vbo, ebo;
	u32 shader_id;
};

funcdef void gfx_init();
funcdef void gfx_deinit();

funcdef void gfx_init_draw_data(Draw_Data *data, Draw type);
funcdef void gfx_deinit_draw_data(Draw_Data *data);

funcdef void gfx_begin(Draw_Data *data, vec2 resolution);
funcdef void gfx_end();

funcdef f32  delta_time();

funcdef void gfx_push_quad(vec2 min, vec2 max, color8 color);

//////////////
// ~gaureesh @NOTE: cursed `defer` construct for c++

template<typename T> struct RemoveReference       { typedef T Type; };
template<typename T> struct RemoveReference<T &>  { typedef T Type; };
template<typename T> struct RemoveReference<T &&> { typedef T Type; };

template<typename T> inline T &&forward(typename RemoveReference<T>::Type &t)  { return static_cast<T &&>(t); }
template<typename T> inline T &&forward(typename RemoveReference<T>::Type &&t) { return static_cast<T &&>(t); }
template<typename T> inline T &&move   (T &&t)                                 { return static_cast<typename RemoveReference<T>::Type &&>(t); }

template<typename F>
struct DeferImpl {
    F f;
    DeferImpl(F &&f) : f(forward<F>(f)) {}
    ~DeferImpl() { f(); }
};
template<typename F> DeferImpl<F> defer_func(F &&f) { return DeferImpl<F>(forward<F>(f)); }

#define TOKEN_PASTE(a, b) a##b
#define DEFER_NAME(base, line) TOKEN_PASTE(base, line)
#define defer(code) auto DEFER_NAME(_defer_, __LINE__) = defer_func([&]() { code; })

//////////////
// ~gaureesh @NOTE: assets

enum class Asset {
	Sprite_Shader,
	Screen_Shader,
	Count,
};

struct Sprite {
	u32 x, y;
	u32 w, h;
	u32 frame_count;
};

funcdef void assets_init();
funcdef void assets_deinit();

funcdef slice<string> asset_fetch_shader_source(Asset shader, Arena *arena);

//////////////
// ~reehan @NOTE: image to binary

struct Image {
    s32 width;
    s32 height;
    s32 channels;
    bytes data;
};

funcdef Image png_to_binary_data(Arena *arena, string path);

#endif // ENGINE_H
