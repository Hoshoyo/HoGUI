#include "immgui.h"
#include "../renderer/renderer_imm.h"
#include "immgui_input.h"
#include "hg_internal.h"
#include "colors.h"

void hg_start(HG_Context* ctx) {
    if(ctx->flags & IMMCTX_HOT_SET)
        ctx->hot.owner = ctx->last_hot;
    ctx->last_hot = -1;
}
void hg_end(HG_Context* ctx) {
}

void hg_end_frame(HG_Context* ctx) {
    if(!(ctx->flags & IMMCTX_HOT_SET))
        reset_hot(ctx);

    ctx->flags = 0;
}

// Layout
bool hg_layout_rectangle_top_down(HG_Context* ctx, vec2* position, r32 *width, r32* height, Clipping_Rect* clipping) {
    if(*height == 0.0f) {
        // default height is 30 px
        *height = 30.0f;
    }

    //if(ctx->current_frame.height < *height) {
    //    return false;
    //}

    clipping->x = ctx->current_frame.x;
    clipping->y = ctx->current_frame.y;
    clipping->z = ctx->current_frame.x + ctx->current_frame.width;
    clipping->w = ctx->current_frame.y + ctx->current_frame.height;

    s32 current_column = ctx->current_frame.current_column;
    s32 vertical_column_count = ctx->current_frame.vertical_column_count;

    r32 column_width = ctx->current_frame.width / (r32)ctx->current_frame.vertical_column_count;

    position->x = ctx->current_frame.x + ((r32)current_column * column_width);
    position->y = ctx->current_frame.y + ctx->current_frame.height - *height;
    *width = column_width;


    // update current frame
    if(ctx->current_frame_set) {
        ctx->current_frame.height -= (*height);
    }

    return true;
}

// -----------------------------------------
// -------------- Slider -------------------
// -----------------------------------------

r32 lerp(r32 v0, r32 v1, r32 t) {
    return (1 - t) * v0 + t * v1;
}

void hg_do_slider(HG_Context* ctx, int id, r32* value, r32 min, r32 max) {
    hg_update(ctx);
    vec2 position = {0};
    r32 width = 0.0f, height = 20.0f;
    Clipping_Rect slider_clipping = {0};
    hg_layout_rectangle_top_down(ctx, &position, &width, &height, &slider_clipping);

    position.x += 8.0f;
    position.y += 9.0f;
    width -= 16.0f;
    Quad_2D base = quad_new(position, width, 2.0f, color_from_hex(pallete1_2));
    renderer_imm_quad(&base);

    r32 range = max - min;
    r32 percentage = (*value - min) / range;

    vec2 handle_pos = position;
    handle_pos.y -= (height / 2.0f);
    handle_pos.x += lerp(0, width - 4.0f, percentage);

    Quad_2D handle = quad_new(handle_pos, 8.0f, height, color_from_hex(pallete1_3));
    renderer_imm_quad(&handle);

    r32 border_width[] = {1.0f, 1.0f, 1.0f, 1.0f};
    vec4 border_color[] = {
        color_from_hex(pallete1_1),
        color_from_hex(pallete1_1),
        color_from_hex(pallete1_1),
        color_from_hex(pallete1_1),
    };
    renderer_imm_border(&handle, border_width, border_color);
}

void hd_do_text(HG_Context* ctx, int id, const char* text, int length) {
    hg_update(ctx);

    vec2 position = {0};
}