#version 330 core
in vec2 o_tcoords;
in vec4 o_color;
in float o_texture_alpha;
in float o_is_text;

in vec4 clipping;
in vec2 position;

out vec4 color;

uniform sampler2D u_text;
uniform bool u_use_texture;

void main(){
#if 1
	if(position.x < clipping.x || position.x > clipping.x + clipping.z) {
		discard;
	}
	if(position.y < clipping.y || position.y > clipping.y + clipping.w) {
		discard;
	}
#endif

	color = mix(o_color, texture(u_text, o_tcoords), o_texture_alpha);
	color = vec4(o_color.rgb, o_color.a * max(color.r, o_is_text));
}