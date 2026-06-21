#include "engine.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_RECT_PACK_IMPLEMENTATION

#include "vendor/stb_image.h"
#include "vendor/stb_rect_pack.h"

global struct 
{
} asset_ctx;


funcdef void
assets_init(Arena *scratch)
{
	//
	// pack sprites
	//

	Temp temp = temp_begin(scratch);
	defer(temp_end(temp));

	stbrp_context ctx;
	slice<stbrp_node> nodes = alloc_slice(temp.arena, stbrp_node, Sprite_Count);
	stbrp_init_target(&ctx, ATLAS_SIZE, ATLAS_SIZE, nodes.raw, Sprite_Count);

	slice<stbrp_rect> rects = alloc_slice(temp.arena, stbrp_rect, Sprite_Count);
	slice<Image> images = alloc_slice(temp.arena, Image, Sprite_Count);

	for (u32 i = 0; i < Sprite_Count; i++) {

		string path = SPRITE_PATHS[i];
		images[i] = asset_fetch_image(temp.arena, (Sprite_Id) i);

		rects[i].id = i;
		rects[i].w = images[i].width + 2;
		rects[i].h = images[i].height + 2; // + 2padding
	}
	stbrp_pack_rects(&ctx, rects.raw, Sprite_Count);

	for (u32 i = 0; i < Sprite_Count; i++) {
		if (!rects[i].was_packed) {
			printf("sprite '%.*s' was not packed\n", S_FMT(SPRITE_PATHS[i]));
			continue;
		}
		gfx_blit_atlas(rects[i].x + 1, rects[i].y + 1, images[i].width, images[i].height, images[i].data, 1, (Sprite_Id) i);
	}
}

funcdef slice<string>
asset_fetch_shader_source(Arena *arena, Shader_Id shader)
{
#if DEBUG_BUILD
    string path = SHADER_PATHS[shader];
    string full_file = string_from_bytes(os_load_entire_file(arena, path));

    s64 v_index = -1;
    s64 f_index = -1;

    for (u64 i = 0; i + 4 <= full_file.len; i++) {
        if (v_index < 0 &&
            memcmp(full_file.raw + i, "#vs\n", 4) == 0) {
            v_index = i;
        }

        if (f_index < 0 &&
            memcmp(full_file.raw + i, "#fs\n", 4) == 0) {
            f_index = i;
        }

        if (v_index >= 0 && f_index >= 0)
            break;
    }

    if (v_index < 0 || f_index < 0) {
        return {};
    }

    s64 i0 = Min(v_index, f_index);
    s64 i1 = Max(v_index, f_index);

    u64 v_slice = (i0 == f_index) ? 1 : 0;


	slice<string> result = alloc_slice(arena, string, 2);
    result[v_slice] = full_file.range(i0 + 4, i1);
    result[1-v_slice] = full_file.range(i1 + 4, full_file.len);
	return result;
#else
# error "asset_fetch_shader_source implementation missing"
#endif
}

//////////////
// ~reehan @NOTE: image to binary

funcdef Image
asset_fetch_image(Arena *arena, Sprite_Id sprite)
{
#if DEBUG_BUILD
	int width;
	int height;
	int channels;

	Temp t = temp_begin(scratch(&arena, 1));
	defer(temp_end(t));

	string path = SPRITE_PATHS[sprite];
	string cpath = string_to_cstring(t.arena,path);

	u8 *data = stbi_load((char*)cpath.raw,&width,&height,&channels,0);
	defer(stbi_image_free(data));

	if (!data){
		return Image{};
	}

	u64 buffer_size = (u64) width * height * channels;
	Image img= {} ;

	img.data = alloc_slice(arena,u8,buffer_size);
	memcpy(img.data.raw, data, buffer_size);

	img.width=width;
	img.height=height;
	img.channels=channels;

	return img;
#else
# error "asset_fetch_image implementation missing"
#endif
}


funcdef bool
game_load_code(Game_Code *code, string path)
{
	code->handle = os_load_library(path);

	if (!code->handle.v)
		return false;

	OS_FileData file_data = os_file_data(path);
	code->timestamp = file_data.timestamp;

	Load_Proc proc  = (Load_Proc) os_load_symbol(code->handle, "game_get_code");
	if (!proc) {
		game_unload_code(code);
		return false;
	}

	Game_Code new_code = proc();

	code->init = new_code.init;
	code->update = new_code.update;

	return true;
}

funcdef void
game_unload_code(Game_Code *code)
{
	os_unload_library(code->handle);
	memset(code, 0x0, sizeof(Game_Code));
}
