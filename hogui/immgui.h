#pragma once
#include "../common.h"

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

bool hg_do_button(void* ctx, int id, const char* text);
void hg_end_frame();