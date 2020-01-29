#pragma once
#include "../common.h"
#include "../renderer/font_render.h"
#include <gm.h>

typedef struct {
    s64 owner;
    int item;
    int index; // represents which of all the hot items is the last (in front)
} uid;

typedef struct {
    r32 width;

    // internal
    r32 current_height;
} HG_Container_Column;

typedef struct {
    r32 x;
    r32 y;
    r32 width;
    r32 height;

    vec4 clipping;
    r32  max_width;
    r32  start_height;

    s32  column_count;
    s32  column_index;
    HG_Container_Column* column_info;
} HG_Frame;

typedef struct {
    uid hot;
    uid active;
    uid previous_active;
    u32 flags;

    uid last_hot;

    bool current_frame_set;
    HG_Frame current_frame;

    Font_Info font_info;
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

typedef enum {
    HG_CONTAINER_TOP_DOWN,
    HG_CONTAINER_BOTTOM_UP,
} HG_Container_Layout;

typedef struct {
    r32  v_scroll_perc;
    r32  h_scroll_perc;

    r32 total_width;
    r32 total_height;

    vec4 clipping;

    HG_Container_Layout layout;

    HG_Frame within_frame;
} HG_Container;

typedef enum {
    HG_INPUT_BOX_EXIT_NONE = 0,
    HG_INPUT_BOX_EXIT_ESCAPE,
    HG_INPUT_BOX_EXIT_ENTER,
    HG_INPUT_BOX_EXIT_TAB,
} HG_Input_Box_Exit;

HG_Context* hg_init();
void hg_start(HG_Context* ctx);
bool hg_end(HG_Context* ctx);   // When input events are handled by GUI return true
void hg_render(HG_Context* ctx);
void hg_set_active(HG_Context* ctx, s64 id, int item);
void hg_reset_active(HG_Context* ctx);
bool hg_active_item(HG_Context* ctx, s64 id, int item);

// Window
void hg_window_begin(HG_Context* ctx, s64 id, vec2* position, r32 width, r32 height, const char* name);

// Container
void hg_column_next(HG_Context* ctx);
void hg_column_previous(HG_Context* ctx);
void hg_do_container_column(HG_Context* ctx, s64 id, HG_Container* container, r32 width, r32 height, HG_Container_Column* columns_info, s32 columns_count, HG_Container_Layout layout_mode);
void hg_do_container(HG_Context* ctx, s64 id, HG_Container* container, r32 width, r32 height, HG_Container_Layout hg_do_container);
void hg_do_container_end(HG_Context* ctx, HG_Container* container);

// Button
bool hg_do_button(HG_Context* ctx, s64 id, int item, const char* text, int text_length, bool enabled);

// Input box
HG_Input_Box_Exit hg_do_input(HG_Context* ctx, s64 id, int item, char* buffer, int buffer_max_length, int* buffer_length, int* cursor_index, int* selection_distance);

// Slider
void hg_do_slider(HG_Context* ctx, s64 id, int item, r32* value, r32 min, r32 max);
r32  hg_do_label_slider(HG_Context* ctx, s64 id, int item, const char* text, int text_length, vec4 color);

// Label
bool hg_do_label(HG_Context* ctx, s64 id, int item, const char* text, int text_length, vec4 color);

// Text
bool hg_do_text_advanced(HG_Context* ctx, s64 id, int item, const char* text, int text_length, int start_index, vec4 color, vec4* out_bbox);
bool hg_do_text(HG_Context* ctx, s64 id, int item, const char* text, int text_length, vec4 color);