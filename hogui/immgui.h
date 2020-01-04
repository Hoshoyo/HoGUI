#pragma once
#include "../common.h"
#include <gm.h>

typedef struct {
    int owner;
    int item;
    int index;
} uid;

typedef struct {
    uid hot;
    uid active;
    u32 flags;
} HG_Context;

void hg_window_begin(HG_Context* ctx, int id, vec2 position, r32 width, r32 height, const char* name);
bool hg_do_button(HG_Context* ctx, int id, const char* text);
bool hg_do_input(HG_Context* ctx, int id, char* buffer, int buffer_max_length, int* buffer_length, int* cursor_index, int* selection_distance);
void hg_end_frame();