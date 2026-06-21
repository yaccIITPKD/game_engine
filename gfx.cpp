#include <stddef.h>

#include "engine.h"
#include "vendor/glad.h"

struct GFX_TextureAtlas {
	u32 gl_id;
};

static struct {
	Arena *persist;

	OS_TimeStamp last_frame_time;
	f32 delta_time;
	vec2 resolution;

	slice<Sprite> sprite_table;

	GFX_TextureAtlas atlas;
} gfx_ctx;

static mat4
mat4_ortho(f32 left, f32 right, f32 bottom, f32 top, f32 near_clip, f32 far_clip)
{
	mat4 m = {0};
	m.m[0]  =  2.0f / (right - left);
	m.m[5]  =  2.0f / (top - bottom);
	m.m[10] = -2.0f / (far_clip - near_clip);
	m.m[12] = -(right + left) / (right - left);
	m.m[13] = -(top + bottom) / (top - bottom);
	m.m[14] = -(far_clip + near_clip)   / (far_clip - near_clip);
	m.m[15] =  1.0f;
	return m;
}

static mat4
mat4_mul(mat4 a, mat4 b)
{
	mat4 out = {0};
	for (int col = 0; col < 4; col++) {
		for (int row = 0; row < 4; row++) {
			f32 sum = 0.0f;
			for (int k = 0; k < 4; k++) {
				sum += a.m[k * 4 + row] * b.m[col * 4 + k];
			}
			out.m[col * 4 + row] = sum;
		}
	}
	return out;
}

static mat4
mat4_camera_view(vec2 position, vec2 offset, f32 scale)
{
	mat4 out = {0};
	out.m[0]  = scale;
	out.m[5]  = scale;
	out.m[10] = 1.0f;
	out.m[15] = 1.0f;
	out.m[12] = (-position.x * scale) + offset.x;
	out.m[13] = (-position.y * scale) + offset.y;
	return out;
}

funcdef void
gfx_draw(Draw_Data *data)
{
	if (data->vertices.len == 0 || data->indices.len == 0) {
		return;
	}

	glUseProgram(data->shader_id);
	glBindVertexArray(data->vao);
	glBindBuffer(GL_ARRAY_BUFFER, data->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->ebo);

	glBufferSubData(
		GL_ARRAY_BUFFER, 0,
		data->vertices.len * sizeof(Vertex),
		data->vertices.raw
	);

	glBufferSubData(
		GL_ELEMENT_ARRAY_BUFFER, 0,
		data->indices.len * sizeof(u16),
		data->indices.raw
	);

	mat4 proj = mat4_ortho(
		0.0f, gfx_ctx.resolution.x,
		gfx_ctx.resolution.y, 0.0f,
		-10000.0f, 1000.0f
	);
	
	mat4 view = mat4_camera_view(
		data->camera.position,
		data->camera.offset,
		data->camera.scale
	);

	mat4 proj_view = mat4_mul(proj, view);
	u32 proj_log = glGetUniformLocation(data->shader_id, "u_proj_view");

	glUniformMatrix4fv(proj_log, 1, GL_FALSE, proj_view.m);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gfx_ctx.atlas.gl_id);
	glUniform1i(glGetUniformLocation(data->shader_id, "u_atlas"), 0);

	GLenum mode = GL_TRIANGLES;
	if (data->primitive == Draw::Lines) {
		mode = GL_LINES;
	}

	glDrawElements(
		mode,
		(GLsizei)data->indices.len,
		GL_UNSIGNED_SHORT,
		nullptr
	);

	data->vertices.len = 0;
	data->indices.len  = 0;
}

funcdef void
gfx_init()
{
	if (!gladLoadGLLoader((GLADloadproc)os_get_gl_proc_address())) {
		fprintf(stderr, "failed to load opengl");
		return;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gfx_ctx.last_frame_time = os_time_now();
	gfx_ctx.persist = arena_make(MB(4));


	//
	// initialize the global texture atlas
	//

	glGenTextures(1, &gfx_ctx.atlas.gl_id);
	glBindTexture(GL_TEXTURE_2D, gfx_ctx.atlas.gl_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_SIZE, ATLAS_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	gfx_ctx.sprite_table = alloc_slice(gfx_ctx.persist, Sprite, Sprite_Count);
}

funcdef void
gfx_deinit()
{
	arena_delete(gfx_ctx.persist);
}

funcdef u32
gfx__compile_shader(int type, string src)
{
	u32 id = glCreateShader(type);

	const char *cstr = (char *)src.raw;
	int len = (int)src.len;

	glShaderSource(id, 1, &cstr, &len);
	glCompileShader(id);

	int success;
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);

	if (!success) {
		char log[1024];
		glGetShaderInfoLog(id, sizeof(log), 0, log);
		printf("Shader compile error:\n%s\n", log);
		return 0;
	}

	return id;
}

funcdef void
gfx_init_draw_data(Draw_Data *data, Draw type, Shader_Id shader)
{
	Temp temp = temp_begin(scratch(0, 0));
	defer(temp_end(temp));

	data->primitive = type;

	slice<Vertex> vertices = alloc_slice(gfx_ctx.persist, Vertex, MAX_VERTICES);
	slice<u16>    indices  = alloc_slice(gfx_ctx.persist, u16, MAX_VERTICES);

	data->vertices = list_make(vertices);
	data->indices  = list_make(indices);
	data->sprites  = gfx_ctx.sprite_table;

	data->draw_proc = gfx_draw;

	data->camera.position = {0, 0};
	data->camera.offset   = {0, 0};
	data->camera.scale    = 1.0f;

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

	slice<string> glsl = asset_fetch_shader_source(temp.arena, shader);
	gfx_reload_shader(data, glsl[0], glsl[1]);
}

funcdef void
gfx_deinit_draw_data(Draw_Data *data)
{
	glDeleteVertexArrays(1, &data->vao);
	glDeleteBuffers(1, &data->vbo);
	glDeleteBuffers(1, &data->ebo);
}

funcdef void
gfx_begin(vec2 resolution)
{
	OS_TimeStamp curr = os_time_now();

	gfx_ctx.delta_time = (f32)os_time_diff(
		gfx_ctx.last_frame_time,
		curr
	).seconds;

	gfx_ctx.last_frame_time = curr;

	gfx_ctx.resolution = resolution;

	glViewport(0, 0, (s32)resolution.x, (s32)resolution.y);

	u32 clear_color = Hex(0x72751BFF);
	f32 a = ((clear_color >> 24) & 0xFF) / 255.0f;
	f32 b = ((clear_color >> 16) & 0xFF) / 255.0f;
	f32 g = ((clear_color >> 8)  & 0xFF) / 255.0f;
	f32 r = (clear_color & 0xFF) / 255.0f;
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

funcdef f32
delta_time()
{
	return gfx_ctx.delta_time;
}

funcdef void
gfx_reload_shader(Draw_Data *data, string vs, string fs)
{
	if (data->shader_id) {
		glDeleteProgram(data->shader_id);
		data->shader_id = 0;
	}

	u32 vs_id = gfx__compile_shader(GL_VERTEX_SHADER,   vs);
	u32 fs_id = gfx__compile_shader(GL_FRAGMENT_SHADER, fs);

	assert(vs_id && fs_id);

	data->shader_id = glCreateProgram();
	glAttachShader(data->shader_id, vs_id);
	glAttachShader(data->shader_id, fs_id);
	glLinkProgram(data->shader_id);

	glDeleteShader(vs_id);
	glDeleteShader(fs_id);
}

funcdef void
gfx_blit_atlas(u32 x, u32 y, u32 w, u32 h, bytes pixels, u32 frames, Sprite_Id sprite)
{
	assert(x + w <= ATLAS_SIZE && "blit out of bounds (x)");
	assert(y + h <= ATLAS_SIZE && "blit out of bounds (y)");

	glBindTexture(GL_TEXTURE_2D, gfx_ctx.atlas.gl_id);
	glTexSubImage2D(
		GL_TEXTURE_2D, 0,
		(GLint)x, (GLint)y,
		(GLsizei)w, (GLsizei)h,
		GL_RGBA, GL_UNSIGNED_BYTE,
		pixels.raw
	);

	gfx_ctx.sprite_table[sprite] = Sprite {
		x, y,
		w, h,
		frames
	};
}
