#include "game.h"


funcdef vec4
draw_quad(Draw_Data *data, vec2 min, vec2 max, color8 color)
{
	if (data->vertices.len + 4 > data->vertices.capacity ||
		data->indices.len + 6 > data->indices.capacity)
	{
		data->draw_proc(data);
	}

	Sprite sprite = data->sprites[Sprite_White];

	f32 width  = (f32) sprite.w / sprite.frames;
	f32 height = sprite.h;


	vec2 uv0 = { (f32) sprite.x, (f32) sprite.y };
	vec2 uv1 = { (f32) (sprite.x + width), (f32) (sprite.y + sprite.h)};

	Vertex vertices[4] = {
		{ { min.x, min.y, 0.0f }, { uv0.x, uv0.y }, color },
		{ { max.x, min.y, 0.0f }, { uv1.x, uv0.y }, color },
		{ { max.x, max.y, 0.0f }, { uv1.x, uv1.y }, color },
		{ { min.x, max.y, 0.0f }, { uv0.x, uv1.y }, color },
	};

	u16 base = (u16)data->vertices.len;
	u16 indices[6] = {
		(u16) (base + 0),
		(u16) (base + 1),
		(u16) (base + 2),
		(u16) (base + 2),
		(u16) (base + 3),
		(u16) (base + 0),
	};

	append_slice(&data->vertices, { vertices, 4 } );
	append_slice(&data->indices,  { indices, 6 } );

	return { min.x, min.y, max.x, max.y };
}


funcdef vec4
draw_sprite(Draw_Data *data, vec2 pos, Align align, Sprite_Id id, bool y_sort, bool x_flip, u32 frame, color8 tint)
{
	if (data->vertices.len + 4 > data->vertices.capacity ||
		data->indices.len + 6 > data->indices.capacity)
	{
		data->draw_proc(data);
	}

	Sprite sprite = data->sprites[id];

	f32 width  = (f32) sprite.w / sprite.frames;
	f32 height = sprite.h;

	u32 frame_x = sprite.x + (u32)(frame * width);

	vec2 uv0 = { (f32) frame_x, (f32) sprite.y };
	vec2 uv1 = { (f32) (frame_x + width), (f32) (sprite.y + sprite.h)};

	vec2 offset = ALIGN_OFFSET[align];

	offset.x *= width;
	offset.y *= height;

	vec2 min = pos - offset;
	vec2 max = min + vec2 { width, height };

	if (x_flip) {
		f32 tmp = uv0.x;
		uv0.x = uv1.x;
		uv1.x = tmp;
	}

	f32 z = y_sort ? pos.y * 0.001f : 0.0f;

	Vertex vertices[4] = {
		{ { min.x, min.y, z }, { uv0.x, uv0.y }, tint },
		{ { max.x, min.y, z }, { uv1.x, uv0.y }, tint },
		{ { max.x, max.y, z }, { uv1.x, uv1.y }, tint },
		{ { min.x, max.y, z }, { uv0.x, uv1.y }, tint },
	};


	u16 base = (u16)data->vertices.len;
	u16 indices[6] = {
		(u16) (base + 0),
		(u16) (base + 1),
		(u16) (base + 2),
		(u16) (base + 2),
		(u16) (base + 3),
		(u16) (base + 0),
	};

	append_slice(&data->vertices, { vertices, 4 } );
	append_slice(&data->indices,  { indices, 6 } );

	return { min.x, min.y, max.x, max.y };
}

