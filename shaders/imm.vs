#version 330 core
layout(location = 0) in vec3 v_vertex;
layout(location = 1) in vec2 v_tcoords;
layout(location = 2) in vec4 v_color;
layout(location = 3) in float v_texture_alpha;
layout(location = 4) in float v_is_text;
layout(location = 5) in vec4 v_clipping;

out vec2 o_tcoords;
out vec4 o_color;
out float o_texture_alpha;
out float o_is_text;
out vec4 clipping;
out vec2 position;

uniform mat4 u_projection = mat4(1.0);
uniform mat4 u_translation = mat4(1.0);

void main(){
	gl_Position = u_projection * u_translation * vec4(v_vertex.xy, 0.0, 1.0);
	position = v_vertex.xy;
	o_tcoords = v_tcoords;
	o_color = v_color;
	o_texture_alpha = v_texture_alpha;
	o_is_text = v_is_text;

	clipping = v_clipping;
}