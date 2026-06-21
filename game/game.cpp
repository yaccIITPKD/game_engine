#include "game.h"

#include "../base.cpp"

#include "draw.cpp"

funcdef vec2
normalize(vec2 v) 
{
	f32 l = (f32) sqrt(v.x * v.x + v.y * v.y);;

	if (l <= 1e-6)
		return vec2 {0,0};

	return v.scale(1.0f/l);
}

funcdef void
game_init(Game_State *state) {
}

funcdef void
game_update(Game_State *state, const OS_Input& input, Draw_Data *draw_data, f32 dt) {
	vec2& player_pos = state->player_pos;

	const f32 MOVE_SPEED = 140;

	vec2 delta = {};
	static u32 frame = 0;
	static bool right = false;

	bool d = input.key_state[Key_D] & Key_Down;
	bool a = input.key_state[Key_A] & Key_Down;
	bool s = input.key_state[Key_S] & Key_Down;
	bool w = input.key_state[Key_W] & Key_Down;

	if (d && !a) { delta.x = 1; frame = 2; right = true; }
	if (a && !d) { delta.x = -1; frame = 2; right = false; }
	if (w && !s) { delta.y = -1; frame = 1; }
	if (s && !w) { delta.y = +1; frame = 0; }

	delta = normalize(delta);
	player_pos += delta.scale(dt * MOVE_SPEED);
	draw_data->camera.scale = 3.0f;
	draw_data->camera.position += (player_pos - draw_data->camera.position).scale(dt * 10.0f);

	//
	// a demo scene
	//

	draw_sprite(draw_data, {-32,10},  Bottom_Center, Sprite_Box, true);
	draw_sprite(draw_data, {54,-18},  Bottom_Center, Sprite_Box, true);
	draw_sprite(draw_data, {-71,40},  Bottom_Center, Sprite_Box, true);
	draw_sprite(draw_data, {88,52},   Bottom_Center, Sprite_Box, true);

	draw_sprite(draw_data, {-110,-30}, Bottom_Center, Sprite_Pillar1, true);
	draw_sprite(draw_data, {62,-95},   Bottom_Center, Sprite_Pillar2, true);
	draw_sprite(draw_data, {-45,-120}, Bottom_Center, Sprite_Pillar3, true);
	draw_sprite(draw_data, {130,15},   Bottom_Center, Sprite_Pillar1, true);

	draw_sprite(draw_data, {-150,120}, Bottom_Center, Sprite_Tree1, true);

	draw_sprite(draw_data, player_pos + vec2{3,-2}, Bottom_Center, Sprite_Player, true, false, 3, Hex(0x00000077));
	draw_sprite(draw_data, player_pos, Bottom_Center, Sprite_Player, true, right, frame);
}

export_fn Game_Code
game_get_code() {
	Game_Code result = {};
	result.init = game_init;
	result.update = game_update;
	return result;
}
