#pragma once
#include "../common.h"
#include <gm.h>

typedef struct {
    int owner;
    int item;
    int index;
} uid;

typedef struct {
    r32 x;
    r32 y;
    r32 width;
    r32 height;

    r32 starting_width;
    r32 starting_height;

    s32 vertical_column_count;
    s32 current_column;
} HG_Frame;

typedef struct {
    uid hot;
    uid active;
    u32 flags;

    bool current_frame_set;
    HG_Frame current_frame;
} HG_Context;

typedef enum {
    HG_WINDOW,
    HG_LIST,
} HG_Type;

typedef struct {
    vec2 position;
    r32 width;
    r32 height;
} HG_Window;

typedef struct {
    HG_Type type;
    union {
        HG_Window window;
    };
} HG_State;

void hg_window_begin(HG_Context* ctx, int id, vec2 position, r32 width, r32 height, const char* name, s32 vertical_column_count);
void hg_window_next_column(HG_Context* ctx);
void hg_window_previous_column(HG_Context* ctx);
bool hg_do_button(HG_Context* ctx, int id, const char* text, int text_length);
bool hg_do_input(HG_Context* ctx, int id, char* buffer, int buffer_max_length, int* buffer_length, int* cursor_index, int* selection_distance);
void hg_end_frame();