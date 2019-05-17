#pragma once
#include "../common.h"
#include "../renderer/renderer_imm.h"
#include <gm.h>
#include "../font/font_render.h"
#include "../event.h"

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
    HOGUI_WINDOW_FLAG_GLOBAL           = FLAG(0),
    HOGUI_WINDOW_FLAG_TOPDOWN          = FLAG(1),
    HOGUI_WINDOW_FLAG_DISABLE_STACKING = FLAG(2),
    HOGUI_WINDOW_FLAG_CLIP_CHILDREN    = FLAG(3),
    HOGUI_WINDOW_FLAG_CONSTRAIN_X      = FLAG(4),
    HOGUI_WINDOW_FLAG_CONSTRAIN_Y      = FLAG(5),
    HOGUI_WINDOW_FLAG_LOCK_MOVE_X      = FLAG(6),
    HOGUI_WINDOW_FLAG_LOCK_MOVE_Y      = FLAG(7),
};

enum {
    HOGUI_WINDOW_TEMP_FLAG_HOVERED        = FLAG(0),
    HOGUI_WINDOW_TEMP_FLAG_MOUSE_LOCKED   = FLAG(1),
    HOGUI_WINDOW_TEMP_FLAG_HOVERED_BORDER = FLAG(2),
};

enum {
    HOGUI_SELECTING_BORDER_LEFT   = FLAG(0),
    HOGUI_SELECTING_BORDER_RIGHT  = FLAG(1),
    HOGUI_SELECTING_BORDER_TOP    = FLAG(2),
    HOGUI_SELECTING_BORDER_BOTTOM = FLAG(3),
};

typedef struct HoGui_Window_t {
    const char* name;
    
    u32  flags, temp_flags, border_flags;
    r32  width, height;
    vec2 position, absolute_position;
    vec4 bg_color, hover_color;
    r32  border_size[4];  // left, right, top, bottom
    vec4 border_color[4]; // left, right, top, bottom

    Scope* scope_at;
    Scope  scope_defined;

    struct HoGui_Window_t** children;
} HoGui_Window;


int hogui_init();
int hogui_render(Font_Info*);
int hogui_delete_window(HoGui_Window* w);
HoGui_Window* hogui_new_window(HoGui_Window* in_info, HoGui_Window* parent);

int hogui_input(Event* e);
int hogui_update();

void hogui_test();