#pragma once
#include "../common.h"
#include "../renderer/renderer_imm.h"
#include <gm.h>
#include "../font/font_render.h"

typedef void* HoGui_Node;

typedef enum {
    HOGUI_TYPE_NONE,
    HOGUI_TYPE_WINDOW,
} HoGui_Type;

typedef struct Scope_t{
    s32 id;
    s32 element_count;

    struct Scope_t* parent;
    struct HoGui_Window* defining_window;

    Clipping_Rect clipping;

    // Info
    r32 max_x, min_x;
    r32 max_y, min_y;
} Scope;

enum {
    HOGUI_WINDOW_FLAG_GLOBAL = FLAG(0),
    HOGUI_WINDOW_FLAG_TOPDOWN = FLAG(1),
    HOGUI_WINDOW_FLAG_DISABLE_STACKING = FLAG(2),
    HOGUI_WINDOW_FLAG_CLIP_CHILDREN = FLAG(3),
};

typedef struct HoGui_Window_t {
    u32  flags;
    r32  width, height;
    vec2 position, absolute_position;
    vec4 bg_color, hover_color;

    Scope* scope_at;
    Scope  scope_defined;

    struct HoGui_Window_t** children;
} HoGui_Window;


int hogui_init();
int hogui_render(Font_Info*);
int hogui_delete_window(HoGui_Window* w);
HoGui_Window* hogui_new_window(HoGui_Window* in_info, HoGui_Window* parent);