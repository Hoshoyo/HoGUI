#version 330 core

out vec4 out_color;

in vec2 texture_coords;
in vec4 color;

uniform sampler2D text;

void main()
{
	vec4 sampled = vec4(color.xyz, texture(text, texture_coords).r * color.a);
	out_color = sampled;
}