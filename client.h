//////////
// ~gaureesh @NOTE: code shared by both
// engine and game.

#ifndef CLIENT_H
#define CLIENT_H

//////////////////////////////////////////////////////////
//           User Defined Asset Path Tables             //
//////////////////////////////////////////////////////////

#define Def_Sprites \
	X(Sprite_White, "./sprite/white.png") \
	X(Sprite_Player, "./sprite/player.png") \
	X(Sprite_Box, "./sprite/box.png") \
	X(Sprite_GroundTiles, "./sprite/ground_tiles.png") \
	X(Sprite_Tree1, "./sprite/tree1.png") \
	X(Sprite_Tree2, "./sprite/tree2.png") \
	X(Sprite_Tree3, "./sprite/tree3.png") \
	X(Sprite_Pillar1, "./sprite/pillar1.png") \
	X(Sprite_Pillar2, "./sprite/pillar2.png") \
	X(Sprite_Pillar3, "./sprite/pillar3.png") \

#define Def_Shaders \
	X(Shader_Sprite, "./shaders/sprite.glsl") \
	X(Sprite_Screen, "./shaders/screen.glsl")



//////////
// ~gaureesh @NOTE: base types

#include <stdint.h> // for fixed size integers.
#include <assert.h> // for assert macro.
#include <string.h> // for memmove, memcpy etc. ( not for cstd string functions ).
#include <stdarg.h> // for va_args.
#include <stdio.h>  // standard io operations.
#include <math.h>

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

    vec2 operator+(const vec2 &rhs) const {
        return {x + rhs.x, y + rhs.y};
    }

    vec2 operator-(const vec2 &rhs) const {
        return {x - rhs.x, y - rhs.y};
    }

    vec2 &operator+=(const vec2 &rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    vec2 &operator-=(const vec2 &rhs) {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

	inline vec2 scale(f32 scalar) {
		return { x * scalar, y * scalar };
	}
};

struct vec3 {
	f32 x, y, z;
};

struct vec4 {
	f32 x, y, z, w; 

	vec2 xy() const { return {x, y}; }
	vec2 yz() const { return {y, z}; }
	vec2 zw() const { return {z, w}; }
};

typedef struct {
	f32 m[16];
} mat4;

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

#if OS_Windows
# define export_fn extern "C" __declspec(dllexport)
#else
# define export_fn extern "C" __attribute__((visibility("default")))
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

#define Hex(x) byteswap_u32((u32)(x))

//////////////
// ~gaureesh @NOTE: os

struct OS_Handle { uintptr_t v; };

// @NOTE: we are keepin the key
// count limited for api simplicity
enum Keys : u8 {
    Key_None = 0,
	Key_Any,

    Key_W,
    Key_A,
    Key_S,
    Key_D,

    Key_Arrow_U,
    Key_Arrow_D,
    Key_Arrow_L,
    Key_Arrow_R,

    Key_Space,
    Key_Enter,
    Key_E,
    Key_Q,
    Key_F,
    Key_R,

    Key_Shift,
    Key_Ctrl,

    Key_Escape,
    Key_Tab,

    Key_Count,
};

enum Key_State : u8 {
	Key_Press   = 1 << 0, // the key was pressed this frame
	Key_Down    = 1 << 1, // the key is currently down
	Key_Release = 1 << 2, // the key was released this frame
};

struct OS_Input {
	u8 key_state[Key_Count];

	vec2 cursor_pos;
	vec2 cursor_delta;
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

typedef u64 OS_TimeStamp; // in nanoseconds
struct OS_TimeDuration {
	f64 seconds;
	f64 milliseconds;
	f64 microseconds;
};

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

//////////////
// ~gaureesh @NOTE: render interface

const u64 MAX_VERTICES = 4098;
const u64 ATLAS_SIZE   = 4098;
const u64 MAX_SPRITES  = 1024;

typedef u32 color8; // in 0xFFBBGGAA format

enum Align : u32 {
	Top_Left,
	Top_Center,
	Top_Right,

	Mid_Left,
	Mid_Center,
	Mid_Right,

	Bottom_Left,
	Bottom_Center,
	Bottom_Right,
};

global const vec2 
ALIGN_OFFSET[] {
	vec2 {0.0f, 0.0f},
	vec2 {0.5f, 0.0f},
	vec2 {1.0f, 0.0f},

	vec2 {0.0f, 0.5f},
	vec2 {0.5f, 0.5f},
	vec2 {1.0f, 0.5f},

	vec2 {0.0f, 1.0f},
	vec2 {0.5f, 1.0f},
	vec2 {1.0f, 1.0f},
};

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

funcdef inline vec2
world_to_screen(const Camera &camera, vec2 world)
{
    return (world - camera.position).scale(camera.scale) + camera.offset;
}

funcdef inline vec2
screen_to_world(const Camera &camera, vec2 screen)
{
    return (screen - camera.offset).scale(1.0f / camera.scale) + camera.position;
}

struct Draw_Data;
typedef void (*Graphics_Draw_Proc)(Draw_Data *);

enum Sprite_Id : u32 {
#define X(name,path) name,
	Def_Sprites
#undef X
	Sprite_Count
};

global const string 
SPRITE_PATHS[Sprite_Count] = {
#define X(name, path) S(path),
	Def_Sprites
#undef X
};

enum Shader_Id : u32  {
#define X(name,path) name,
	Def_Shaders
#undef X
	Shader_Count
};

global const string
SHADER_PATHS[Shader_Count] = {
#define X(name, path) S(path),
	Def_Shaders
#undef X
};

struct Sprite {
	u32 x, y;
	u32 w, h;
	u32 frames;
};

struct Draw_Data {
	Draw primitive;
	Camera camera;

	list<Vertex> vertices;
	list<u16>    indices;
	slice<Sprite> sprites;

	Graphics_Draw_Proc draw_proc; // to flush draw data
								  // from user code
	// gl specific data
	u32 vao, vbo, ebo;
	u32 shader_id;
	u32 texture_id;
};

////////////////////////////////////////////////
// interface from game to the engine

struct Game_State {
	Camera main_camera;
	vec2 player_pos;
};

typedef void (*Init_Proc)(Game_State *);
typedef void (*Update_Proc)(Game_State *, const OS_Input&, Draw_Data *, f32 dt);

struct Game_Code {
	OS_Handle   handle;
	Init_Proc   init;
	Update_Proc update;
	u64         timestamp;
};

typedef Game_Code (*Load_Proc)();

#endif
