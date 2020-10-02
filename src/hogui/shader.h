#pragma once
#include <ho_gl.h>

unsigned int shader_new_lines();
unsigned int shader_load_from_buffer(const char* vert_shader, const char* frag_shader, int vert_length, int frag_length);