#include "immgui.h"
#include "../renderer/renderer_imm.h"
#include "immgui_input.h"
#include "hg_internal.h"
#include "colors.h"

void hg_window_next_column(HG_Context* ctx) {
    ctx->current_frame.current_column = (ctx->current_frame.current_column + 1) % ctx->current_frame.vertical_column_count;
    ctx->current_frame.height = ctx->current_frame.starting_height;
}

void hg_window_previous_column(HG_Context* ctx) {
    ctx->current_frame.current_column -= 1;
    if(ctx->current_frame.current_column == 0)
        ctx->current_frame.current_column = ctx->current_frame.vertical_column_count - 1;
    ctx->current_frame.height = ctx->current_frame.starting_height;
}

static void snap_position(vec2* position, s32 snap_distance) {
    // snap
    r32 diffx = (r32)((s32)position->x % snap_distance);
    r32 diffy = (r32)((s32)position->y % snap_distance);
    if(diffx != 0.0f) {
        if(diffx / 2.0f < snap_distance / 2.0f) {
            position->x -= diffx;
        } else {
            position->x += diffx;
        }
    }
    if(diffy != 0.0f) {
        if(diffy / 2.0f < snap_distance / 2.0f) {
            position->y -= diffy;
        } else {
            position->y += diffy;
        }
    }
}

static void 
window_render(HG_Context* ctx, bool is_hot, r32 header_height, int id, vec2 position, r32 width, r32 height, const char* name, s32 vertical_column_count) {
    // Positioning values
    r32  border_width = 1.0f;
    vec4 color_border = color_from_hex(pallete1_3);
    vec4 header_color = color_from_hex(pallete1_2);

    // Render main window
    Quad_2D q = quad_new(position, width, height, color_from_hex(pallete1_1));
    renderer_imm_quad(&q);

    // Render window border
    renderer_imm_outside_border_v2(q, border_width, color_border);

    // Render header body
    if(is_hot) header_color.r -= 0.3f;
    Quad_2D header = quad_new((vec2){position.x, position.y + height}, width, header_height, header_color);
    renderer_imm_quad(&header);

    // Render header border
    renderer_imm_outside_border_v2(header, border_width, color_border);
}

void hg_window_begin(HG_Context* ctx, int id, vec2* in_position, r32 width, r32 height, const char* name, s32 vertical_column_count) {
    //hg_update(ctx);
    ctx->inside_container = false;

    // Positioning parameters
    vec2 position = *in_position;
    vec2 mouse_position = input_mouse_position();
    bool is_hot = hot(ctx, id);
    r32  header_height = 25.0f;
    r32  snap_grid = 30.0f;

    if(active(ctx, id)) {
        // Dragging
        vec2 down_position = input_mouse_button_down_pos(MOUSE_LEFT_BUTTON);
        position.x += (mouse_position.x - down_position.x);
        position.y += (mouse_position.y - down_position.y);
        
        if(input_is_key_down(GLFW_KEY_LEFT_SHIFT))
            snap_position(&position, snap_grid);

        if(input_mouse_button_went_up(MOUSE_LEFT_BUTTON, 0, 0)) {
            in_position->x = position.x;
            in_position->y = position.y;
            set_active(ctx, ctx->previous_active.owner, 0);
        }
    } else if(is_hot) {
        if(input_mouse_button_went_down(MOUSE_LEFT_BUTTON, 0, 0)) {
            set_active(ctx, id, 0);
        }
    }

    vec4 header_clipping = (vec4){position.x, position.y + height, width, header_height};
    if(input_inside(input_mouse_position(), header_clipping)) {
        set_hot(ctx, id, 0);
    }

    // Draw window
    window_render(ctx, is_hot, header_height, id, position, width, height, name, vertical_column_count);

    ctx->current_frame.x = position.x;
    ctx->current_frame.y = position.y;
    ctx->current_frame.width = width;
    ctx->current_frame.height = height;
    ctx->current_frame.starting_width = width;
    ctx->current_frame.starting_height = height;
    ctx->current_frame_set = true;
    ctx->current_frame.vertical_column_count = vertical_column_count;
    ctx->current_frame.current_column = 0;

    //renderer_imm_debug_box(position.x, position.y, width, height, (vec4){1.0f, 1.0f, 0.0f, 1.0f});
}







/*
    CONTAINER
*/

static const int HG_SCROLL_VERTICAL = 1;
static const int HG_SCROLL_HORIZONTAL = 2;

static Clipping_Rect
scroll_render(HG_Context* ctx, int id, r32 x, r32 y, r32 width, r32 height) {
    // Design parameters
    vec4 color_scroll = color_from_hex(pallete1_3);
    vec4 color_scroll_border = color_from_hex(pallete1_4);
    r32  border_width = 1.0f;

    Clipping_Rect clipping = {x, y, width, height};

    // Render scroll bar
    Quad_2D q = quad_new_clipped((vec2){x, y}, width, height, color_scroll, clipping);
    renderer_imm_quad(&q);

    // Render scroll bar border
    renderer_imm_border_clipped_simple(q, border_width, color_scroll_border, clipping);

    return clipping;
}

void 
hg_do_container(HG_Context* ctx, int id, r32 width, r32 height, r32* total_width, r32* total_height, r32* vscroll_percentage, r32* hscroll_percentage) {
    // Positioning parameters
    vec2 clipping_position = {0};
    r32  vertical_scroll_size = 10.0f;
    r32  horizontal_scroll_size = 10.0f;

    r32 vpercentage = (vscroll_percentage) ? *vscroll_percentage : 0.0f;
    r32 hpercentage = (hscroll_percentage) ? *hscroll_percentage : 0.0f;
    vec2 mouse_position = input_mouse_position();
    if(active_item(ctx, id, HG_SCROLL_VERTICAL) || active_item(ctx, id, HG_SCROLL_HORIZONTAL)) {
        // skip
    } else if(hot_item(ctx, id, HG_SCROLL_VERTICAL)) {
        if(input_mouse_button_went_down(MOUSE_LEFT_BUTTON, 0, 0)) {
            set_active(ctx, id, HG_SCROLL_VERTICAL);
        }
    } else if(hot_item(ctx, id, HG_SCROLL_HORIZONTAL)) {
        if(input_mouse_button_went_down(MOUSE_LEFT_BUTTON, 0, 0)) {
            set_active(ctx, id, HG_SCROLL_HORIZONTAL);
        }
    }

    // Setup percentages width and height
    width = ctx->current_frame.width * ((width == 0) ? 1.0f : width);
    height = ctx->current_frame.height * ((height == 0) ? 1.0f : height);

    // If total height is bigger than the space we have, create a vertical scroll bar,
    // reserve space for it.
    if(total_height && *total_height > height) {
        width -= vertical_scroll_size;
    } else {
        vertical_scroll_size = 0.0f;
    }
    if(total_width && *total_width > width) {
        height -= horizontal_scroll_size;
    } else {
        horizontal_scroll_size = 0.0f;
    }
    if(total_width && *total_width == 0.0f) *total_width = width;
    if(total_height && *total_height == 0.0f) *total_height = height;

    clipping_position = (vec2){ctx->current_frame.x, ctx->current_frame.y + ctx->current_frame.height - height};
    // Update current context clipping
    ctx->current_frame.clipping = (vec4){clipping_position.x, clipping_position.y, width, height};
    ctx->inside_container = true;

    // Debug render clipping area
    //renderer_imm_debug_box(clipping_position.x, clipping_position.y, width, height, (vec4){1.0f, 1.0f, 0.0f, 1.0f});    

    // Update current context user space
    ctx->current_frame.width = *total_width;
    ctx->current_frame.height = *total_height;
    vec2 user_position = (vec2){clipping_position.x, clipping_position.y - (*total_height - height)};
    r32 vsheight = (height * height) / *total_height;
    r32 hswidth = (width * width) / *total_width;

    if(active_item(ctx, id, HG_SCROLL_VERTICAL)) {
        vec2 down_position = input_mouse_button_down_pos(MOUSE_LEFT_BUTTON);
        r32 diff_y = (mouse_position.y - down_position.y);
        vpercentage += (diff_y / (height - vsheight));
        vpercentage = MIN(1.0f, MAX(0.0f, vpercentage));

        if(input_mouse_button_went_up(MOUSE_LEFT_BUTTON, 0, 0)) {
            if(vscroll_percentage) *vscroll_percentage = vpercentage;
            reset_active(ctx);
        }
    } else if(active_item(ctx, id, HG_SCROLL_HORIZONTAL)) {
        vec2 down_position = input_mouse_button_down_pos(MOUSE_LEFT_BUTTON);
        r32 diff_x = (mouse_position.x - down_position.x);
        hpercentage += (diff_x / (width - hswidth));
        hpercentage = MIN(1.0f, MAX(0.0f, hpercentage));

        if(input_mouse_button_went_up(MOUSE_LEFT_BUTTON, 0, 0)) {
            if(hscroll_percentage) *hscroll_percentage = hpercentage;
            reset_active(ctx);
        }
    }

    // Calculate the heights for both user space and the vertical scroll bar representing it
    r32 clip_y = lerp(clipping_position.y, clipping_position.y + height - vsheight, vpercentage);
    r32 user_y = lerp(user_position.y, user_position.y + *total_height - height, 1.0f - vpercentage);

    r32 clip_x = lerp(clipping_position.x, clipping_position.x + width - hswidth, hpercentage);
    r32 user_x = lerp(user_position.x, user_position.x - (*total_width - width), hpercentage);

    // Set the current user space context to the correct position.
    ctx->current_frame.y = user_y;
    ctx->current_frame.x = user_x;

    // Debug user area
    //renderer_imm_debug_box(user_x, user_y, *total_width, *total_height, (vec4){1.0f, 0.0f, 0.0f, 1.0f});

    Clipping_Rect vscroll_box = scroll_render(ctx, id, user_position.x + width, clip_y, vertical_scroll_size, vsheight);
    Clipping_Rect hscroll_box = scroll_render(ctx, id, clip_x, clipping_position.y - horizontal_scroll_size, hswidth, horizontal_scroll_size);

    if(input_inside(input_mouse_position(), vscroll_box)) {
        set_hot(ctx, id, HG_SCROLL_VERTICAL);
    }

    if(input_inside(input_mouse_position(), hscroll_box)) {
        set_hot(ctx, id, HG_SCROLL_HORIZONTAL);
    }
}