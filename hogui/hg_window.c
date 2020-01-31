#include "hg_ui.h"
#include "../renderer/renderer_imm.h"
#include "hg_input.h"
#include "hg_internal.h"
#include "colors.h"

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
window_render(HG_Context* ctx, bool is_hot, r32 header_height, int id, vec2 position, r32 width, r32 height, const char* name) {
    // Positioning values
    r32  border_width = 1.0f;
    vec4 color_border = color_from_hex(pallete1_3);
    vec4 header_color = color_from_hex(pallete1_2);
    vec4 color_window = color_from_hex(pallete1_1);
    vec4 color_title  = color_from_hex(pallete1_1);
    r32  header_border_width[] = { 1.0f, 1.0f, 1.0f, 0.0f };
    vec4 header_border_color[] = {color_border, color_border, color_border, color_border};
    color_window.a = 0.8f;

    // Render main window
    Quad_2D q = quad_new(position, width, height, color_window);
    renderer_imm_quad(&q);

    // Render window border
    renderer_imm_outside_border_v2(q, border_width, color_border);

    // Render header body
    if(is_hot) header_color.r -= 0.3f;
    Quad_2D header = quad_new((vec2){position.x, position.y + height}, width, header_height, header_color);
    renderer_imm_quad(&header);

    // Render header border
    renderer_imm_outside_border(&header, header_border_width, header_border_color);

    // Render window_name
    if(name) {
        Text_Render_Info info = text_prerender(&ctx->font_info, name, strlen(name), 0, 0, 0);
        r32 ydiff = header_height / 2.0f - info.height / 2.0f;
        text_render(&ctx->font_info, name, strlen(name), 0, (vec2){position.x + 4.0f, position.y + height + ydiff},
            (vec4){position.x, position.y + height, width, header_height}, color_title);
    }
}

void hg_window_begin(HG_Context* ctx, s64 id, vec2* in_position, r32 width, r32 height, const char* name) {
    //hg_update(ctx);

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
            hg_set_active(ctx, ctx->previous_active.owner, 0);
        }
    } else if(is_hot) {
        if(input_mouse_button_went_down(MOUSE_LEFT_BUTTON, 0, 0)) {
            hg_set_active(ctx, id, 0);
        }
    }

    vec4 header_clipping = (vec4){position.x, position.y + height, width, header_height};
    if(input_inside(input_mouse_position(), header_clipping)) {
        set_hot(ctx, id, 0);
    }

    ctx->current_frame.x = position.x;
    ctx->current_frame.y = position.y;
    ctx->current_frame.width = width;
    ctx->current_frame.height = height;
    ctx->current_frame.clipping = (vec4){position.x, position.y, width, height};

    ctx->current_frame_set = true;
    
    // Draw window
    window_render(ctx, is_hot, header_height, id, position, width, height, name);

    //renderer_imm_debug_box(position.x, position.y, width, height, (vec4){1.0f, 0.0f, 1.0f, 1.0f});
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

void hg_column_next(HG_Context* ctx) {
    ctx->current_frame.column_index = (ctx->current_frame.column_index + 1) % ctx->current_frame.column_count;
}

void hg_column_previous(HG_Context* ctx) {
    ctx->current_frame.column_index--;
    if(ctx->current_frame.column_index < 0) ctx->current_frame.column_index = ctx->current_frame.column_count;
}

void hg_do_container_column(HG_Context* ctx, s64 id, HG_Container* container, r32 width, r32 height, HG_Container_Column* columns_info, s32 columns_count, HG_Container_Layout layout_mode) {
    vec2 layout_position = {0};
    Clipping_Rect last_clipping = ctx->current_frame.clipping;
    hg_layout_rectangle_top_down(ctx, &layout_position, &width, &height, &ctx->current_frame.clipping);

    // If total height is bigger than the space we have, create a vertical scroll bar,
    // reserve space for it.
    r32 vertical_scroll_size = (container->total_height > height) ? 10.0f : 0.0f;
    r32 horizontal_scroll_size = (container->total_width > width) ? 10.0f : 0.0f;
    height -= horizontal_scroll_size;
    width -= vertical_scroll_size;

    // Save current context
    container->within_frame = ctx->current_frame;
    container->within_frame.clipping = last_clipping;

    Clipping_Rect layout = (Clipping_Rect){layout_position.x, layout_position.y, width, height};
    Clipping_Rect clipping = clipping_rect_merge(layout, ctx->current_frame.clipping);

    // Calculate the vertical scroll height
    r32 vsheight = (height * height) / container->total_height;
    r32 vpercentage = container->v_scroll_perc;

    // Calculate the horizontal scroll width
    r32 hswidth = (width * width) / container->total_width;
    r32 hpercentage = container->h_scroll_perc;

    {
        vec2 mouse_position = input_mouse_position();

        // Handle scroll input
        if(active_item(ctx, id, HG_SCROLL_VERTICAL)) {
            // When vertical scroll is active 
            vec2 down_position = input_mouse_button_down_pos(MOUSE_LEFT_BUTTON);
            r32 diff_y = (mouse_position.y - down_position.y);
            vpercentage += (diff_y / (height - vsheight));
            vpercentage = MIN(1.0f, MAX(0.0f, vpercentage));

            if(input_mouse_button_went_up(MOUSE_LEFT_BUTTON, 0, 0)) {
                container->v_scroll_perc = vpercentage;
                hg_reset_active(ctx);
            }
        } else if(active_item(ctx, id, HG_SCROLL_HORIZONTAL)) {
            // When horizontal scroll is active
            vec2 down_position = input_mouse_button_down_pos(MOUSE_LEFT_BUTTON);
            r32 diff_x = (mouse_position.x - down_position.x);
            hpercentage += (diff_x / (width - hswidth));
            hpercentage = MIN(1.0f, MAX(0.0f, hpercentage));

            if(input_mouse_button_went_up(MOUSE_LEFT_BUTTON, 0, 0)) {
                container->h_scroll_perc = hpercentage;
                hg_reset_active(ctx);
            }
        } else if(hot_item(ctx, id, HG_SCROLL_VERTICAL)) {
            // When vertical scroll is hot but not active
            if(input_mouse_button_went_down(MOUSE_LEFT_BUTTON, 0, 0)) {
                hg_set_active(ctx, id, HG_SCROLL_VERTICAL);
            }
        } else if(hot_item(ctx, id, HG_SCROLL_HORIZONTAL)) {
            // When horizontal scroll is hot but not active
            if(input_mouse_button_went_down(MOUSE_LEFT_BUTTON, 0, 0)) {
                hg_set_active(ctx, id, HG_SCROLL_HORIZONTAL);
            }
        }
    }
    
    r32 gap = container->total_height - height;
    layout.y -= (gap - horizontal_scroll_size);
    clipping.y += horizontal_scroll_size;

    r32 clip_y = lerp(clipping.y, clipping.y + height - vsheight, vpercentage);
    r32 clip_x = lerp(clipping.x, clipping.x + width - hswidth, hpercentage);

    r32 user_y = lerp(layout.y, layout.y + container->total_height - height, 1.0f - vpercentage);
    r32 user_x = lerp(layout.x, layout.x - (container->total_width - width), hpercentage);

    if(layout_mode == HG_CONTAINER_TOP_DOWN && vertical_scroll_size == 0.0f)
        user_y = user_y - gap; 

    Clipping_Rect vscroll_box = scroll_render(ctx, id, layout.x + width, clip_y, vertical_scroll_size, vsheight - 1.0f);
    Clipping_Rect hscroll_box = scroll_render(ctx, id, clip_x, clipping.y - horizontal_scroll_size, hswidth - 1.0f, horizontal_scroll_size);

    // Update current frame
    ctx->current_frame.clipping = clipping;
    ctx->current_frame.x = user_x;
    ctx->current_frame.y = user_y;
    ctx->current_frame.width = container->total_width;
    ctx->current_frame.height = container->total_height;
    ctx->current_frame.max_width = 0.0f;
    ctx->current_frame.column_info = columns_info;
    ctx->current_frame.column_count = columns_count;
    ctx->current_frame.column_index = 0;

    if(input_inside(input_mouse_position(), vscroll_box)) {
        set_hot(ctx, id, HG_SCROLL_VERTICAL);
    }

    if(input_inside(input_mouse_position(), hscroll_box)) {
        set_hot(ctx, id, HG_SCROLL_HORIZONTAL);
    }
#if 0
    renderer_imm_debug_box(user_x, user_y, container->total_width, container->total_height, (vec4){1,0,1,1});
    renderer_imm_debug_clipping(clipping, (vec4){1,0,0,1});
#endif
}

void hg_do_container(HG_Context* ctx, s64 id, HG_Container* container, r32 width, r32 height, HG_Container_Layout layout_mode) {
    hg_do_container_column(ctx, id, container, width, height, 0, 0, layout_mode);
}

void hg_do_container_end(HG_Context* ctx, HG_Container* container) {
    container->total_width = ctx->current_frame.max_width;
    container->total_height -= ctx->current_frame.height;

    if(ctx->current_frame.column_count > 0) {
        for(s32 i = 0; i < ctx->current_frame.column_count; ++i) {
            ctx->current_frame.column_info[i].current_height = container->total_height;
        }
    }

    ctx->current_frame = container->within_frame;
}