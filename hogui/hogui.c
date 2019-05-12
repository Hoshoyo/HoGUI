#include "hogui.h"
#include <float.h>
#include "../renderer/renderer_imm.h"
#include <light_array.h>
#include <assert.h>

#define Unimplemented assert(0)

static HoGui_Window global_window;
static s32          global_id;

int 
hogui_init() {
    global_window.children = array_new(HoGui_Window*);
    global_window.scope_at = &global_window.scope_defined;
    global_window.flags = HOGUI_WINDOW_FLAG_GLOBAL;

    // global_scope
    global_window.scope_defined.defining_window = (struct HoGui_Window*)&global_window;
    global_window.scope_defined.id = global_id++;
    global_window.scope_defined.parent = 0;

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

bool
hogui_scope_flag_is_set_recursive(Scope* scope, u32 flag) {
    if(scope->id == 0) {
        return hogui_scope_flag_is_set(scope, flag);
    } else {
        if(hogui_scope_flag_is_set(scope, flag)) 
            return true;
        return hogui_scope_flag_is_set_recursive(scope->parent, flag);
    }
}

vec2
hogui_window_position(HoGui_Window* w) {
    if(w->flags & HOGUI_WINDOW_FLAG_GLOBAL) {
        return w->position;
    }
    HoGui_Window* parent = (HoGui_Window*)w->scope_at->defining_window;
    if(parent) {
        vec2 result = {0};
        if(hogui_scope_flag_is_set(w->scope_at, HOGUI_WINDOW_FLAG_TOPDOWN)) {
            // height of the parent
            result.y += ((HoGui_Window*)w->scope_at->defining_window)->height;
            result.y -= w->height;
        }
        return gm_vec2_add(gm_vec2_add(w->position, hogui_window_position(parent)), result);
    } else {
        return w->position;
    }
}

int
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

int
hogui_render_window(HoGui_Window* w) {
    Scope* current_scope = w->scope_at;

    // Render the window itself
    vec2 base_position = hogui_window_position(w);
    vec2 scope_adjustment = hogui_scope_adjustment(w);
    vec2 position = gm_vec2_add(base_position, scope_adjustment);

    Quad_2D q = quad_new_clipped(position, w->width, w->height, w->bg_color, current_scope->clipping);
    renderer_imm_quad(&q);

    // Update scope
    current_scope->max_x += (w->position.x + w->width);
    current_scope->max_y += (w->position.y + w->height);

    // Merge the clipping downwards
    if(w->flags & HOGUI_WINDOW_FLAG_CLIP_CHILDREN) {
        Clipping_Rect c = clipping_rect_new(position.x, position.y, w->width, w->height);
        w->scope_defined.clipping = clipping_rect_merge(current_scope->clipping, c);
    }

    // Render children
    if(w->children) {
        for(u64 i = 0; i < array_length(w->children); ++i) {
            hogui_render_window(w->children[i]);
        }
        // Reset scope
        hogui_reset_scope(&w->scope_defined);
    }
}

int
hogui_render() {
    hogui_reset_scope(&global_window.scope_defined);
    for(u64 i = 0; i < array_length(global_window.children); ++i) {
        HoGui_Window* w = global_window.children[i];
        hogui_render_window(w);
        // Reset scope
        hogui_reset_scope(w->scope_at);
    }
}