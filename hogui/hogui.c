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

    int width, height;
    window_get_size(&width, &height);
    global_window.width = (r32)width;
    global_window.height = (r32)height;

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

bool
hogui_point_inside_border(vec2 p, HoGui_Window* w, r32 slack, u32* out_flags) {
	r32 left = w->absolute_position.x;
	r32 right = w->absolute_position.x + w->width;
	r32 bot = w->absolute_position.y;
	r32 top = w->absolute_position.y + w->height;

	bool touching_left = (p.x >= left - slack) && (p.x <= left + slack) && (p.y >= bot - slack) && (p.y <= top + slack);
	bool touching_right = (p.x >= right - slack) && (p.x <= right + slack) && (p.y >= bot - slack) && (p.y <= top + slack);
	bool touching_top = (p.y >= top - slack) && (p.y <= top + slack) && (p.x >= left - slack) && (p.x <= right + slack);
	bool touching_bot = (p.y >= bot - slack) && (p.y <= bot + slack) && (p.x >= left - slack) && (p.x <= right + slack);

	if (touching_left) *out_flags |= HOGUI_SELECTING_BORDER_LEFT;
	if (touching_right) *out_flags |= HOGUI_SELECTING_BORDER_RIGHT;
	if (touching_bot) *out_flags |= HOGUI_SELECTING_BORDER_BOTTOM;
	if (touching_top) *out_flags |= HOGUI_SELECTING_BORDER_TOP;

	return touching_left || touching_right || touching_bot || touching_top;
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

    if(position.x < 0.0f && w->flags & HOGUI_WINDOW_FLAG_CONSTRAIN_X)
        position.x = 0.0f;
    if(position.y < 0.0f && w->flags & HOGUI_WINDOW_FLAG_CONSTRAIN_Y)
        position.y = 0.0f;

    // Limit the right and top borders
    if(w->flags & HOGUI_WINDOW_FLAG_CONSTRAIN_X && (scope_width - w->width > 0) && (position.x > scope_width - w->width)) {
        position.x = scope_width - w->width;
    }
    if(w->flags & HOGUI_WINDOW_FLAG_CONSTRAIN_Y && (scope_height - w->height > 0) && (position.y > scope_height - w->height)) {
        position.y = scope_height - w->height;
    }

    r32 absolute_x = global_locked_diff.x + w->absolute_position.x;
    r32 absolute_y = global_locked_diff.y + w->absolute_position.y;

    // Calculate the offset from the mouse lock position
    r32 offset_x = absolute_x - mouse_current_pos.x + diff.x;
    r32 offset_y = absolute_y - mouse_current_pos.y + diff.y;

    if(start_pos.x < position.x) {
        // moving right
        if(offset_x > 0.0f) {
            position.x = start_pos.x;
        }
    } else if(start_pos.x > position.x) {
        // moving left
        if(offset_x < 0.0f) {
            position.x = start_pos.x;
        }
    }
    // When the mouse goes outside of the scope, wait until it is again
    // in its locked offset to move the window again 
    if(((HoGui_Window*)w->scope_at->defining_window)->flags & HOGUI_WINDOW_FLAG_TOPDOWN) {
        if(start_pos.y < position.y && offset_y < 0.0f) {
            // moving up
            position.y = start_pos.y;
        } else if (start_pos.y > position.y && offset_y > 0.0f) {
            // moving down
            position.y = start_pos.y;
        }
    } else {
        if(start_pos.y > position.y && offset_y < 0.0f) {
            // moving up
            position.y = start_pos.y;
        } else if (start_pos.y < position.y && offset_y > 0.0f) {
            // moving down
            position.y = start_pos.y;
        }
    }

    if(w->flags & HOGUI_WINDOW_FLAG_LOCK_MOVE_X) {
        position.x = start_pos.x;
    }
    if(w->flags & HOGUI_WINDOW_FLAG_LOCK_MOVE_Y) {
        position.y = start_pos.y;
    }

    w->position = position;
    return gm_vec2_subtract(position, start_pos);
}

static vec2
hogui_resize(HoGui_Window* w, vec2 mouse_diff, r32 min_width, r32 min_height) {
    vec2 p = {0};
    vec2 diff = {0};

    if(w->locking_border_flags & HOGUI_LOCKING_BORDER_LEFT) {
        // Limit the resizing to min_width
        if(w->width - mouse_diff.x - min_width < 0.0f) {
            mouse_diff.x = -min_width + w->width;
        }
        // Apply change to window
        w->width -= mouse_diff.x;
        w->position.x += mouse_diff.x;
        p.x += mouse_diff.x;
    } 
    if(w->locking_border_flags & HOGUI_LOCKING_BORDER_RIGHT) {
        // Limit the resizing to min_width
        if(w->width + mouse_diff.x - min_width < 0.0f) {
            mouse_diff.x = min_width - w->width;
        }
        // Apply change to window
        w->width += mouse_diff.x;
    }

    if(w->flags & HOGUI_WINDOW_FLAG_TOPDOWN) {
        if(w->locking_border_flags & HOGUI_LOCKING_BORDER_TOP) {
            // Limit the resizing to min_height
            if(w->height + mouse_diff.y - min_height < 0.0f) {
                mouse_diff.y = min_height - w->height;
            }
            // Apply change to window
            w->height += mouse_diff.y;
        }
        if(w->locking_border_flags & HOGUI_LOCKING_BORDER_BOTTOM) {
            // Limit the resizing to the size of the window
            if(w->height - mouse_diff.y - min_height < 0.0f) {
                mouse_diff.y = -min_height + w->height;
            }
            // Apply change to window
            w->height -= mouse_diff.y;
            w->position.y += mouse_diff.y;
            p.y += mouse_diff.y;
        }
    } else {
        if(w->locking_border_flags & HOGUI_LOCKING_BORDER_TOP) {
            if(w->height - mouse_diff.y - min_height < 0.0f) {
                mouse_diff.y = -min_height + w->height;
            }
            // Apply change to window
            w->height -= mouse_diff.y;
            w->position.y += mouse_diff.y;
        }
        if(w->locking_border_flags & HOGUI_LOCKING_BORDER_BOTTOM) {
            if(w->height + mouse_diff.y - min_height < 0.0f) {
                mouse_diff.y = min_height - w->height;
            }
            // Apply change to window
            w->height += mouse_diff.y;
            p.y -= mouse_diff.y;
        }

    }
    return p;
}

static int
hogui_render_window(HoGui_Window* w, Font_Info* font_info) {
    Scope* current_scope = w->scope_at;
    vec2 position = w->absolute_position;

    bool resizing = (w->locking_border_flags && w->temp_flags & HOGUI_WINDOW_TEMP_FLAG_MOUSE_LOCKED) &&
        (((w->locking_border_flags & HOGUI_LOCKING_BORDER_LEFT || w->locking_border_flags & HOGUI_LOCKING_BORDER_RIGHT) && w->flags & HOGUI_WINDOW_FLAG_RESIZEABLE_H) ||
        ((w->locking_border_flags & HOGUI_LOCKING_BORDER_BOTTOM || w->locking_border_flags & HOGUI_LOCKING_BORDER_TOP) && w->flags & HOGUI_WINDOW_FLAG_RESIZEABLE_V));

    // Change window position according to movement of the mouse when locked
    if(w->temp_flags & HOGUI_WINDOW_TEMP_FLAG_MOUSE_LOCKED) {
        vec2 mouse_diff = hogui_window_mouse_locked_diff(w);

        // Update current window position
        vec2 diff = {0};
        if(resizing) {
            diff = hogui_resize(w, mouse_diff, 10.0f, 20.0f);
        } else {
            diff = hogui_update_position(w, mouse_diff);
        }
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

    // Merge the clipping downwards (meaning, all child windows will clip within the bounds of this one).
    Clipping_Rect c = {0.0f, 0.0f, FLT_MAX, FLT_MAX};
    if(w->flags & HOGUI_WINDOW_FLAG_CLIP_CHILDREN) {
        c = clipping_rect_new(position.x, position.y, w->width, w->height);
    }
    w->scope_defined.clipping = clipping_rect_merge(current_scope->clipping, c);

    // Render current window
    Quad_2D q = quad_new_clipped(position, w->width, w->height, color, w->scope_defined.clipping);
    Quad_2D* q_rendered = renderer_imm_quad(&q);

    //renderer_imm_debug_text(font_info, position, "Hello");
    renderer_imm_debug_text_clipped(font_info, position, w->scope_defined.clipping, "Hello");

    // Render border
    if(w->border_size[0] + w->border_size[1] + w->border_size[2] + w->border_size[3] > 0.0f) {
        vec4 border_color[4] = {0};
        border_color[0] = w->border_color[0];
        border_color[1] = w->border_color[1];
        border_color[2] = w->border_color[2];
        border_color[3] = w->border_color[3];
        
        if((w->temp_flags & HOGUI_WINDOW_TEMP_FLAG_HOVERED_BORDER) || (w->locking_border_flags && w->temp_flags & HOGUI_WINDOW_TEMP_FLAG_MOUSE_LOCKED)) {
            vec4 red = (vec4) {1.0f, 0.0f, 0.0f, 1.0f};
            if(w->border_flags & HOGUI_SELECTING_BORDER_LEFT || w->locking_border_flags & HOGUI_LOCKING_BORDER_LEFT) {
                border_color[0] = red;
            }
            if(w->border_flags & HOGUI_SELECTING_BORDER_RIGHT || w->locking_border_flags & HOGUI_LOCKING_BORDER_RIGHT) {
                border_color[1] = red;
            }
            if(w->border_flags & HOGUI_SELECTING_BORDER_TOP || w->locking_border_flags & HOGUI_LOCKING_BORDER_TOP) {
                border_color[3] = red;
            }
            if(w->border_flags & HOGUI_SELECTING_BORDER_BOTTOM || w->locking_border_flags & HOGUI_LOCKING_BORDER_BOTTOM) {
                border_color[2] = red;
            }
        }
        renderer_imm_border(&q, w->border_size, border_color);
    }

    // Render children
    if(w->children) {
        for(u64 i = 0; i < array_length(w->children); ++i) {
            hogui_render_window(w->children[i], font_info);
        }
    }

    // Reset temporary flags
    w->temp_flags = 0;
    w->border_flags = 0;

    // Reset Scope info
    hogui_reset_scope(&w->scope_defined);
}

// External function to render all windows in the global scope
int
hogui_render(Font_Info* font_info) {
    for(u64 i = 0; i < array_length(global_window.children); ++i) {
        HoGui_Window* w = global_window.children[i];
        hogui_render_window(w, font_info);
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
    u32 flags = 0;
    if(hogui_point_inside_border(mouse_current_pos, w, 3.0f, &flags)) {
        w->border_flags = flags;
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
    
    if(mouse_locked && !global_locked && !global_hovered) {
        global_locked = &global_window;
    }

    if(global_hovered) {
        global_hovered->temp_flags |= HOGUI_WINDOW_TEMP_FLAG_HOVERED;
        if(global_hovered->border_flags) {
            global_hovered->temp_flags |= HOGUI_WINDOW_TEMP_FLAG_HOVERED_BORDER;
        }
        if(mouse_locked && !global_locked) {
            global_locked = global_hovered;
            global_locked_diff = gm_vec2_subtract(mouse_current_pos, global_locked->absolute_position);
            if(global_locked->temp_flags & HOGUI_WINDOW_TEMP_FLAG_HOVERED_BORDER) {
                global_locked->locking_border_flags |= global_locked->border_flags;
            }
        }
        global_hovered = 0;
    }
    if(global_locked) {
        global_locked->temp_flags |= HOGUI_WINDOW_TEMP_FLAG_MOUSE_LOCKED;
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
                if(global_locked)
                    global_locked->locking_border_flags = 0;
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
		.flags = 
            HOGUI_WINDOW_FLAG_TOPDOWN|
            HOGUI_WINDOW_FLAG_CLIP_CHILDREN|
            HOGUI_WINDOW_FLAG_CONSTRAIN_X|HOGUI_WINDOW_FLAG_CONSTRAIN_Y|
            HOGUI_WINDOW_FLAG_RESIZEABLE_H|HOGUI_WINDOW_FLAG_RESIZEABLE_V,
		.width = 520.0f,
		.height = 768.0f,
		.position = (vec2){100.0f, 100.0f},
		.bg_color = (vec4){0.5f, 0.5f, 0.56f, 1.0f},
        .border_size = {2.0f, 2.0f, 2.0f, 2.0f},
        .border_color = {(vec4){0, 0, 0, 1}, (vec4){0, 0, 0, 1}, (vec4){0, 0, 0, 1}, (vec4){0, 0, 0, 1}},
	};
	HoGui_Window* m = hogui_new_window(&w, 0);

	HoGui_Window** ws = array_new(HoGui_Window*);
	for(int i = 0; i < 1; ++i) {
		HoGui_Window ww = {
            .name = "Child",
			.flags = 
                HOGUI_WINDOW_FLAG_CLIP_CHILDREN|
                HOGUI_WINDOW_FLAG_CONSTRAIN_X|
                HOGUI_WINDOW_FLAG_CONSTRAIN_Y|
                HOGUI_WINDOW_FLAG_RESIZEABLE_H|HOGUI_WINDOW_FLAG_RESIZEABLE_V,
			.position = (vec2){20.0f, 10.0f},
			.width = 300.0f,
			.height = 300.0f,
			.bg_color = (vec4){0.0f, 1.0f, 0.0f, 1.0f},
		};
		HoGui_Window* mm = hogui_new_window(&ww, m);
		array_push(ws, mm);

        HoGui_Window www = {
            .name = "GrandChild",
			.flags = 
                HOGUI_WINDOW_FLAG_CLIP_CHILDREN|
                HOGUI_WINDOW_FLAG_CONSTRAIN_X|
                HOGUI_WINDOW_FLAG_CONSTRAIN_Y|
                HOGUI_WINDOW_FLAG_LOCK_MOVE_Y,
			.position = (vec2){20.0f, 10.0f},
			.width = 200.0f,
			.height = 40.0f,
			.bg_color = (vec4){0.3f, 0.3f, 0.3f, 1.0f},
		};
        hogui_new_window(&www, mm);
	}
}