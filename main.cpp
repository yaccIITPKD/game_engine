#include "engine.h"


funcdef void
hot_reload_gamecode(Game_Code *code)
{
#if OS_Window
	string lib_path = S("./game.dll");
#elif OS_Linux
	string lib_path = S("./game.so");
#endif

	OS_FileData data = os_file_data(lib_path);

    if (data.timestamp != code->timestamp) {
        game_unload_code(code);

        OS_TimeDuration dur = {};
        dur.milliseconds = 16.0f;
        os_time_sleep(dur);

        bool ok = game_load_code(code, lib_path);
        assert(ok);
    }
}


funcdef void
entry_point(slice<string> args)
{
	Arena *persist = arena_make(MB(4));
	Arena *frame = arena_make(MB(4));

	os_init();
	defer(os_deinit());

	OS_Handle window = os_open_window(S("engine"));
	defer(os_close_window(window));

	gfx_init();
	defer(gfx_deinit());

	Draw_Data draw_data = {};

	gfx_init_draw_data(&draw_data, Draw::Triangles, Shader_Sprite);
	defer(gfx_deinit_draw_data(&draw_data));

	assets_init(frame);

	Game_Code game = {};
	Game_State game_state = {};
	game.timestamp = 0;

	hot_reload_gamecode(&game);

	assert(game.update && game.init);

	game.init(&game_state);

	u32 cursor_x = 1; // one pixel for white
	u32 cursor_y = 0;
	u32 row_height = 0;

	draw_data.sprites[Sprite_Player].frames = 4;
	draw_data.camera.scale = 4.0f;

	while(!os_window_should_close(window))
	{
		arena_free(frame);
		hot_reload_gamecode(&game);

		OS_Input input = os_prepare_frame(window);

		vec2 resolution = os_window_size(window);
		draw_data.camera.offset = resolution.scale(0.5);

		gfx_begin(resolution);

		game.update(&game_state, input, &draw_data, delta_time());

		gfx_draw(&draw_data);
	}
}
