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
    hg_update(ctx);

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
            set_active(ctx, ctx->previous_active.owner);
        }
    } else if(is_hot) {
        if(input_mouse_button_went_down(MOUSE_LEFT_BUTTON, 0, 0)) {
            set_active(ctx, id);
        }
    }

    vec4 header_clipping = (vec4){position.x, position.y + height, width, header_height};
    if(input_inside(input_mouse_position(), header_clipping)) {
        set_hot(ctx, id);
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
}