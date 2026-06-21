#ifndef ENGINE_H
#define ENGINE_H

#include "client.h"

//////////////
// ~gaureesh @NOTE: arena

struct Arena {
	u64   reserved;
	u64   committed;
	u64   used;
};

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

funcdef Arena *scratch(Arena **conflicts, u64 count);

#define alloc_struct(_arr, _T) (_T *) arena_allocate((_arr), nullptr, 0, sizeof(_T), alignof(_T))
#define alloc_slice(_arr, _T, _n) slice<_T> { (_T *) arena_allocate((_arr), nullptr, 0, sizeof(_T) * (_n), alignof(_T)), (u64) (_n) }
#define realloc_slice(_arr, _s, _n) { (decltype((_s).data)) arena_allocate((_arr), (_s).data, sizeof(*(_s).data) * (_s).count, sizeof(*(_s).data) * (_n), alignof(decltype(*(_s).data))), (u64)(_n) }

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

funcdef OS_TimeStamp    os_time_now();
funcdef OS_TimeDuration os_time_diff(OS_TimeStamp t0, OS_TimeStamp t1);
funcdef void            os_time_sleep(OS_TimeDuration duration);
funcdef u64             os_duration_to_ns(OS_TimeDuration d);

funcdef OS_FileData os_file_data(string path);
funcdef Load_Error  os_file_to_buffer(u8 *ptr, u64 len, string path);
funcdef bytes       os_load_entire_file(Arena *arena, string path);
funcdef bool        os_write_to_file(string path, bytes data);

funcdef OS_Handle   os_load_library(string path);
funcdef void       *os_load_symbol(OS_Handle lib, const char *name);
funcdef void        os_unload_library(OS_Handle lib);

funcdef void   os_set_working_dir(string dir);
funcdef string os_get_working_dir(Arena *arena);
funcdef string os_get_exec_directory(Arena *arena);

//////////////
// ~gaureesh @NOTE: string

funcdef string string_copy(Arena *arena, string str);
funcdef string string_from_bytes(bytes data);
funcdef string string_concat(Arena *arena, string a, string b);
funcdef string string_format(Arena *arena, const char *fmt_string, ...);
funcdef string string_from_cstring(Arena *arena, char *cstring);
funcdef string string_to_cstring(Arena *arena, string s);
funcdef slice<string> strings_from_cstrings(Arena *arena, int count, char **cstrings);

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

///////////////////
// ~gaureesh @NOTE: interface from engine to game 

funcdef bool game_load_code(Game_Code *code, string path);
funcdef void game_unload_code(Game_Code *code);

//////////////
// ~gaureesh @NOTE: assets

struct Image {
    s32 width;
    s32 height;
    s32 channels;
    bytes data;
};

funcdef void assets_init(Arena *scratch);

funcdef slice<string> asset_fetch_shader_source(Arena *arena, Shader_Id shader);
funcdef Image         asset_fetch_image(Arena *arena, Sprite_Id sprite);

//////////////////////////////
// ~gaureesh @NOTE: gfx

funcdef void gfx_init();
funcdef void gfx_deinit();

funcdef void gfx_init_draw_data(Draw_Data *data, Draw type, Shader_Id shader);
funcdef void gfx_deinit_draw_data(Draw_Data *data);
funcdef void gfx_blit_atlas(u32 x, u32 y, u32 w, u32 h, bytes pixels, u32 frames, Sprite_Id spritesset);
funcdef void gfx_reload_shader(Draw_Data *data, string vs, string fs);

funcdef void gfx_begin(vec2 resolution);
funcdef void gfx_draw(Draw_Data *data);
funcdef f32  delta_time();

#endif // ENGINE_H
