#include <stddef.h>

#include "engine.h"
#include "vendor/glad.h"

#define MAX_VERTICES 2048


static struct  {
	Arena *persist;
	OS_TimeStamp last_frame_time;
	f32 delta_time;
} gfx_ctx;

funcdef void
gfx_init()
{
	if (!gladLoadGLLoader((GLADloadproc)os_get_gl_proc_address())) {
		fprintf(stderr, "failed to load opengl");
		return;
	}

	gfx_ctx.last_frame_time = os_time_now();
	gfx_ctx.persist = arena_make(MB(4));
}

funcdef void
gfx_deinit()
{
	// for now does nothing
}

funcdef void
gfx_init_draw_data(Draw_Data *data, Draw type)
{
	data->primitive = type;

	slice<Vertex> vertices = alloc_slice(gfx_ctx.persist, Vertex, MAX_VERTICES);
	slice<u16>    indices  = alloc_slice(gfx_ctx.persist, u16, MAX_VERTICES);

	data->vertices = list_make(vertices);
	data->indices  = list_make(indices);

	glGenVertexArrays(1, &data->vao);
	glGenBuffers(1, &data->vbo);
	glGenBuffers(1, &data->ebo);

	glBindVertexArray(data->vao);

	glBindBuffer(GL_ARRAY_BUFFER, data->vbo);
	glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_VERTICES * sizeof(u16), nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texcoords));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void *)offsetof(Vertex, color));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

funcdef void
gfx_deinit_draw_data(Draw_Data *data)
{
    glDeleteVertexArrays(1, &data->vao);
    glDeleteBuffers(1, &data->vbo);
    glDeleteBuffers(1, &data->ebo);
}

funcdef void
gfx_begin()
{
	OS_TimeStamp curr = os_time_now();
	gfx_ctx.delta_time = (f32) os_time_diff(
		gfx_ctx.last_frame_time, curr
	).seconds;
	gfx_ctx.last_frame_time = curr;

	glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

funcdef void
gfx_end()
{
	// for now does nothing
}

funcdef f32 
delta_time()
{
	return gfx_ctx.delta_time;
}
