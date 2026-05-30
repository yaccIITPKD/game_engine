#include "engine.h"

#include <stdio.h>

funcdef void
entry_point(slice<string> args)
{
	os_init();
	defer(os_deinit());

	OS_Handle window = os_open_window(S("engine"));
	defer(os_close_window(window));

	gfx_init();
	defer(gfx_deinit());

	Draw_Data draw_data = {};

	gfx_init_draw_data(&draw_data, Draw::Triangles);
	defer(gfx_deinit_draw_data(&draw_data));

	for (bool q=false; !q; q = os_window_should_close(window))
	{
		OS_Input input = os_prepare_frame(window);
		gfx_begin();



		gfx_end();
	}
}
