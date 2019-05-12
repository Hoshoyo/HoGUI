#pragma once
#include "../common.h"
#include "../renderer/renderer_imm.h"
#include <gm.h>

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
    vec2 position;
    vec4 bg_color;

    Scope* scope_at;
    Scope  scope_defined;

    struct HoGui_Window_t** children;
} HoGui_Window;


int hogui_init();
int hogui_render();
HoGui_Window* hogui_new_window(HoGui_Window* in_info, HoGui_Window* parent);