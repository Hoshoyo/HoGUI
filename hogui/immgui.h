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
void hg_end_frame();