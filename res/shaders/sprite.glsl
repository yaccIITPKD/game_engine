#vs
#version 400 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_uv;
layout (location = 2) in vec4 a_color;

uniform mat4 u_proj_view;

out vec4 v_color;
out vec2 v_uv;

void main() {
	gl_Position = u_proj_view * vec4(a_pos, 1.0);
	v_uv = a_uv;
	v_color = a_color;
}

#fs
#version 400 core

in vec4 v_color;
in vec2 v_uv;

out vec4 Frag_Color;

uniform sampler2D u_atlas;

void main() {
	ivec2 texel = ivec2(floor(v_uv));
	Frag_Color = v_color * texelFetch(u_atlas, texel, 0);

	if (Frag_Color.a <= 0.1)
		discard;
}
