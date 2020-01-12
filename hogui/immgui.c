#include "immgui.h"
#include "../renderer/renderer_imm.h"
#include "immgui_input.h"
#include "hg_internal.h"
#include "colors.h"

void hg_start(HG_Context* ctx) {
    if(ctx->flags & IMMCTX_HOT_SET) {
        ctx->hot.owner = ctx->last_hot.owner;
        ctx->hot.item = ctx->last_hot.item;
    }
    ctx->last_hot.owner = -1;
    ctx->last_hot.item = -1;
    ctx->inside_container = false;
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

    if(ctx->inside_container) {
        *clipping = ctx->current_frame.clipping;
    } else {
        clipping->x = ctx->current_frame.x;
        clipping->y = ctx->current_frame.y;
        clipping->z = ctx->current_frame.starting_width;
        clipping->w = ctx->current_frame.starting_height;
    }

    return true;
}