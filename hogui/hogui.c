#include "hogui.h"
#include <float.h>
#include "../renderer/renderer_imm.h"
#include <light_array.h>
#include <assert.h>
#include "../input.h"

#define Unimplemented assert(0)

static HoGui_Window global_window;
static s32          global_id;

static HoGui_Window* global_hovered;
static HoGui_Window* global_locked;
static vec2          global_locked_diff;

static bool mouse_locked;
static vec2 mouse_lock_pos;
static vec2 mouse_current_pos;

int 
hogui_init() {
    global_window.children = array_new(HoGui_Window*);
    global_window.scope_at = &global_window.scope_defined;
    global_window.flags = HOGUI_WINDOW_FLAG_GLOBAL;

    // global_scope
    global_window.scope_defined.defining_window = (struct HoGui_Window*)&global_window;
    global_window.scope_defined.id = global_id++;
    global_window.scope_defined.parent = 0;

    hogui_test();

    return 0;
}

int
hogui_delete_window(HoGui_Window* w) {
    // delete the children
    if(w->children) {
        for(u64 i = 0; i < array_length(w->children); ++i) {
            hogui_delete_window(w->children[i]);
        }
        array_free(w->children);
    }

    // delete parent reference
    HoGui_Window* parent = (HoGui_Window*)w->scope_at->defining_window;
    for(u64 i = 0; i < array_length(parent->children); ++i) {
        if(parent->children[i] == w) {
            array_remove_ordered(parent->children, i);
        }
    }

    // delete window itself, if it is not the global_window
    if(w->scope_defined.id != 0) {
        free(w);
    }

    return 0;
}

int
hogui_destroy() {
    return hogui_delete_window(&global_window);
}

HoGui_Window*
hogui_new_window(HoGui_Window* in_info, HoGui_Window* parent) {
    HoGui_Window* w = calloc(1, sizeof(*w));
    memcpy(w, in_info, sizeof(*w));
    
    if(!parent) {
        parent = &global_window;
    }

    if(parent) {
        w->scope_at = &parent->scope_defined;
        w->scope_at->element_count++;
        w->scope_defined.id = global_id++;
        w->scope_defined.element_count = 0;
        w->scope_defined.defining_window = (struct HoGui_Window*)w;
        w->scope_defined.parent = &parent->scope_defined;

        if(!parent->children) {
            parent->children = array_new(HoGui_Window*);
        }
        array_push(parent->children, w);
    }

    return w;
}

bool
hogui_scope_flag_is_set(Scope* scope, u32 flag) {
    return (((HoGui_Window*)scope->defining_window)->flags & flag);
}

static bool
hogui_scope_flag_is_set_recursive(Scope* scope, u32 flag) {
    if(scope->id == 0) {
        return hogui_scope_flag_is_set(scope, flag);
    } else {
        if(hogui_scope_flag_is_set(scope, flag)) 
            return true;
        return hogui_scope_flag_is_set_recursive(scope->parent, flag);
    }
}

static vec2
hogui_window_position(HoGui_Window* w) {
    if(w->flags & HOGUI_WINDOW_FLAG_GLOBAL) {
        return w->position;
    }
    HoGui_Window* parent = (HoGui_Window*)w->scope_at->defining_window;
    if(parent) {
        vec2 result = {0};
        vec2 offset_pos = w->position;
        if(hogui_scope_flag_is_set(w->scope_at, HOGUI_WINDOW_FLAG_TOPDOWN)) {
            // height of the parent
            result.y += ((HoGui_Window*)w->scope_at->defining_window)->height;
            result.y -= w->height;
            offset_pos.y = -offset_pos.y;
        }
        return gm_vec2_add(gm_vec2_add(offset_pos, hogui_window_position(parent)), result);
    } else {
        return w->position;
    }
}

static int
hogui_reset_scope(Scope* scope) {
    scope->clipping = (vec4){0.0f, 0.0f, FLT_MAX, FLT_MAX};
    scope->max_x = 0.0f;
    scope->max_y = 0.0f;
    scope->min_x = 0.0f;
    scope->min_y = 0.0f;
}

vec2
hogui_scope_adjustment(HoGui_Window* w) {
    Scope* scope = w->scope_at;

    if(hogui_scope_flag_is_set(scope, HOGUI_WINDOW_FLAG_TOPDOWN)) {
        return (vec2) {0.0f, -scope->max_y};
    } else {
        return (vec2) {0.0f, scope->max_y};
    }
}

static bool
hogui_window_is_hovered(HoGui_Window* w) {
    r32 mouse_x = mouse_current_pos.x; 
    r32 mouse_y = mouse_current_pos.y;

    return (
        (mouse_x >= w->absolute_position.x && mouse_x <= w->absolute_position.x + w->width) && 
        (mouse_y >= w->absolute_position.y && mouse_y <= w->absolute_position.y + w->height)
    );
}

static vec2
hogui_window_mouse_locked_diff(HoGui_Window* w) {
    vec2 mouse_diff;
    if(w->flags & HOGUI_WINDOW_FLAG_TOPDOWN) {
        mouse_diff = 
            gm_vec2_subtract(mouse_current_pos, mouse_lock_pos);
    } else {
        mouse_diff = 
            (vec2) {mouse_current_pos.x - mouse_lock_pos.x, -(mouse_current_pos.y - mouse_lock_pos.y)};
            //gm_vec2_subtract(mouse_current_pos, mouse_lock_pos);
    }
    
    return mouse_diff;
}

// Return the difference from the current position
static vec2
hogui_update_position(HoGui_Window* w, vec2 diff) {
    r32 scope_width = ((HoGui_Window*)w->scope_at->defining_window)->width;
    r32 scope_height = ((HoGui_Window*)w->scope_at->defining_window)->height;

    vec2 start_pos = w->position;
    // Update current window position
    vec2 position = gm_vec2_add(w->position, diff);
    if(position.x < 0.0f)
        position.x = 0.0f;
    if(position.y < 0.0f)
        position.y = 0.0f;

    if((scope_width - w->width > 0) && (position.x > scope_width - w->width)) {
        position.x = scope_width - w->width;
    }
    if((scope_height - w->height > 0) && (position.y > scope_height - w->height)) {
        position.y = scope_height - w->height;
    }

    r32 abs_x = global_locked_diff.x + w->absolute_position.x;
    r32 offset_x = abs_x - mouse_current_pos.x + diff.x;
    if(offset_x > 0.0f) {
        position.x = start_pos.x;
    }

    w->position = position;
    return gm_vec2_subtract(start_pos, position);
}

static int
hogui_render_window(HoGui_Window* w) {
    Scope* current_scope = w->scope_at;
    vec2 position = w->absolute_position;

    // Change window position according to movement of the mouse when locked
    if(w->temp_flags & HOGUI_WINDOW_TEMP_FLAG_MOUSE_LOCKED) {
        vec2 mouse_diff = hogui_window_mouse_locked_diff(w);

        // Update current window position
        //w->position = gm_vec2_add(w->position, mouse_diff);
        vec2 diff = hogui_update_position(w, mouse_diff);
        position = gm_vec2_add(position, diff);
        
        // Reset mouse lock position to the current, this is because
        // we already updated the position of the window, if we dont do
        // this, the window will move again.
        mouse_lock_pos = mouse_current_pos;
    }

    vec4 color = w->bg_color;
    if(w->temp_flags & HOGUI_WINDOW_TEMP_FLAG_HOVERED) {
        color = gm_vec4_add(w->bg_color, (vec4){-0.2f, -0.2f, -0.2f, 0.0f});
    }

    // Render current window
    Quad_2D q = quad_new_clipped(position, w->width, w->height, color, current_scope->clipping);
    Quad_2D* q_rendered = renderer_imm_quad(&q);

    // Merge the clipping downwards (meaning, all child windows will clip within the bounds of this one).
    Clipping_Rect c = {0.0f, 0.0f, FLT_MAX, FLT_MAX};
    if(w->flags & HOGUI_WINDOW_FLAG_CLIP_CHILDREN) {
        c = clipping_rect_new(position.x, position.y, w->width, w->height);
    }
    w->scope_defined.clipping = clipping_rect_merge(current_scope->clipping, c);

    // Render children
    if(w->children) {
        for(u64 i = 0; i < array_length(w->children); ++i) {
            hogui_render_window(w->children[i]);
        }
    }

    // Reset temporary flags
    w->temp_flags = 0;

    // Reset Scope info
    hogui_reset_scope(&w->scope_defined);
}

// External function to render all windows in the global scope
int
hogui_render(Font_Info* font_info) {
    for(u64 i = 0; i < array_length(global_window.children); ++i) {
        HoGui_Window* w = global_window.children[i];
        hogui_render_window(w);
    }

    // Debug information
    renderer_imm_debug_text(font_info, (vec2){5.0f, 5.0f}, "Mouse: %.0f %.0f", mouse_current_pos.x, mouse_current_pos.y);
}

// Calculates the positioning of all windows, also the current
// hovered and mouse locked window. It uses scope information
// to organize them inside the current scope (defined by the 
// window they are in). The scope is reset after everything 
// is calculated.
static int
hogui_update_window(HoGui_Window* w) {
    Scope* current_scope = w->scope_at;

    // Calculate position
    vec2 base_position    = hogui_window_position(w);
    vec2 scope_adjustment = hogui_scope_adjustment(w);
    vec2 position         = gm_vec2_add(base_position, scope_adjustment);

    // When locked, mouse moves before the hovering check,
    // so we have to set the hovering window to the one locked always.
    if(global_locked) {
        global_hovered = global_locked;
    } else {
        if(hogui_window_is_hovered(w)) {
            global_hovered = w;
        }
    }

    w->absolute_position = position;

    // Update scope
    current_scope->max_x += (w->position.x + w->width);
    current_scope->max_y += (w->position.y + w->height);

    // Update children
    if(w->children) {
        for(u64 i = 0; i < array_length(w->children); ++i) {
            hogui_update_window(w->children[i]);
        }
    }
    // Reset scope
    hogui_reset_scope(&w->scope_defined);
}

int
hogui_update() {
    hogui_reset_scope(&global_window.scope_defined);
    for(u64 i = 0; i < array_length(global_window.children); ++i) {
        HoGui_Window* w = global_window.children[i];
        hogui_update_window(w);
    }
    if(global_hovered) {
        global_hovered->temp_flags = HOGUI_WINDOW_TEMP_FLAG_HOVERED;
        if(mouse_locked && !global_locked) {
            global_locked = global_hovered;
            global_locked_diff = gm_vec2_subtract(mouse_current_pos, global_locked->absolute_position);
            printf("lock difference from position: %.0f %.0f\n", global_locked_diff.x, global_locked_diff.y);
        }
        global_hovered = 0;
    }
    if(global_locked) {
        global_locked->temp_flags = HOGUI_WINDOW_TEMP_FLAG_MOUSE_LOCKED;
    }
}

int 
hogui_input(Event* e) {
    switch(e->type) {
        case EVENT_MOUSE_INPUT:{
            if(e->mouse.type == MOUSE_BUTTON_PRESS) {
                mouse_locked = true;
                input_get_mouse_pos(&mouse_lock_pos.x, &mouse_lock_pos.y);
            } else if(e->mouse.type == MOUSE_BUTTON_RELEASE) {
                mouse_locked = false;
                global_locked = 0;
            } else if(e->mouse.type == MOUSE_POSITION) {
                input_get_mouse_pos(&mouse_current_pos.x, &mouse_current_pos.y);
            }
        }break;
        default: break;
    }
}

void
hogui_test() {
    HoGui_Window w = {
        .name = "Main",
		.flags = HOGUI_WINDOW_FLAG_TOPDOWN|HOGUI_WINDOW_FLAG_CLIP_CHILDREN,
		.width = 520.0f,
		.height = 768.0f,
		.position = (vec2){100.0f, 100.0f},
		.bg_color = (vec4){1.0f, 0.0f, 0.0f, 1.0f},
	};
	HoGui_Window* m = hogui_new_window(&w, 0);

	HoGui_Window** ws = array_new(HoGui_Window*);
	for(int i = 0; i < 10; ++i) {		
		HoGui_Window ww = {
            .name = "Child",
			.flags = HOGUI_WINDOW_FLAG_CLIP_CHILDREN,
			.position = (vec2){20.0f, 10.0f},
			.width = 200.0f,
			.height = 40.0f,
			.bg_color = (vec4){0.0f, 1.0f, 0.0f, 1.0f},
		};
		HoGui_Window* mm = hogui_new_window(&ww, m);
		array_push(ws, mm);
	}
}