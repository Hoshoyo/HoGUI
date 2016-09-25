#version 330 core

layout(location = 0) in vec4 position;

out vec2 texture_coords;
out vec4 color;

uniform mat4 MVP = mat4(1.0);
uniform vec4 font_color = vec4(1.0, 1.0, 1.0, 1.0);

void main()
{
	gl_Position = MVP * vec4(position.xy, 0.0, 1.0);
	texture_coords = position.zw;
	color = font_color;
}