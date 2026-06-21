#ifndef GAME_H
#define GAME_H

#include "../client.h"




funcdef vec4 draw_quad(Draw_Data *data, vec2 min, vec2 max, color8 color);
funcdef vec4 draw_sprite(
		Draw_Data *data, 
		vec2 pos, Align align, Sprite_Id id, 
		bool x_flip = false, 
		bool y_sorting = false,
		u32 frame=0,
		color8 tint = Hex(0xFFFFFFFF)
);

#endif
