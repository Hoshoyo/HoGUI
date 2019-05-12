#pragma once
#include "../common.h"

typedef u32 Shader;

typedef struct {
    u32 shader;
    char* vertex_shader_path;
    char* fragment_shader_path;
} Shader_HL;

// Basic functions
Shader shader_load(const char* vertex_shader_path, const char* fragment_shader_path);
Shader shader_load_from_buffer(const s8* vert_shader, const s8* frag_shader, int vert_length, int frag_length);
void shader_delete(Shader shader);

Shader_HL shader_hl_load(const char* vertex_shader_path, const char* fragment_shader_path);
void shader_hl_update(Shader_HL* shl);
void shader_hl_destroy(const Shader_HL* shl);