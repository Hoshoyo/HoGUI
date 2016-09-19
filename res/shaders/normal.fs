#version 330 core

out vec4 out_color;

in vec2 texture_coords;

uniform sampler2D text;

void main()
{
	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, texture_coords).r);
	out_color = sampled;
}