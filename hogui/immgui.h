#pragma once
#include "../common.h"
#include <gm.h>

typedef struct {
    int owner;
    int item;
    int index; // represents which of all the hot items is the last (in front)
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
    uid previous_active;
    u32 flags;

    int last_hot;

    bool current_frame_set;
    HG_Frame current_frame;
} HG_Context;

typedef enum {
    HG_WINDOW,
    HG_LIST,
} HG_Type;

typedef enum {
    HG_NONE = 0,
    HG_ACTIVE,
    HG_HOT,
} HG_Mode;

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

void hg_start(HG_Context* ctx);
//void hg_end(HG_Context* ctx);

void hg_window_begin(HG_Context* ctx, int id, vec2* position, r32 width, r32 height, const char* name, s32 vertical_column_count);
void hg_window_next_column(HG_Context* ctx);
void hg_window_previous_column(HG_Context* ctx);

bool hg_do_button(HG_Context* ctx, int id, const char* text, int text_length, bool enabled);

bool hg_do_input(HG_Context* ctx, int id, char* buffer, int buffer_max_length, int* buffer_length, int* cursor_index, int* selection_distance);

void hg_do_slider(HG_Context* ctx, int id, r32* value, r32 min, r32 max);

bool hg_do_label(HG_Context* ctx, int id, const char* text, int text_length, vec4 color);

bool hg_do_text(HG_Context* ctx, int id, const char* text, int text_length, vec4 color);