#include "hg_ui.h"
#include "../renderer/renderer_imm.h"
#include "hg_input.h"
#include "hg_internal.h"
#include "colors.h"
#include "../event.h"
#include <float.h>

HG_Context* hg_init() {
    HG_Context* ctx = calloc(1, sizeof(HG_Context));
    ctx->active.owner = -1;
    ctx->active.item = -1;
    ctx->hot.owner = -1;
    ctx->hot.item = -1;
    ctx->previous_active.owner = -1;
    ctx->previous_active.item = -1;
    ctx->last_hot.owner = -1;
    ctx->last_hot.item = -1;

    if(font_load(OS_DEFAULT_FONT, &ctx->font_info, 16) != FONT_LOAD_OK) {
        printf("Could not load font %s", OS_DEFAULT_FONT);
        free(ctx);
        return 0;
    }
    return ctx;
}

void hg_render(HG_Context* ctx) {
    renderer_imm_enable_blending();
    glDisable(GL_DEPTH_TEST);
    renderer_imm_flush(ctx->font_info.atlas_full_id);
    glEnable(GL_DEPTH_TEST);
    renderer_imm_disable_blending();
}

void hg_start(HG_Context* ctx) {
    if(ctx->flags & IMMCTX_HOT_SET) {
        ctx->hot.owner = ctx->last_hot.owner;
        ctx->hot.item = ctx->last_hot.item;
    }
    ctx->last_hot.owner = -1;
    ctx->last_hot.item = -1;
}

bool hg_end(HG_Context* ctx) {
    if(input_mouse_button_went_down(MOUSE_LEFT_BUTTON, 0,0)) {
        if (ctx->hot.owner == -1) {
            hg_reset_active(ctx);
        }
        return (ctx->hot.owner != -1);
    }
    return (ctx->active.owner != -1);
}

void hg_set_active(HG_Context* ctx, s64 id, int item) {
    ctx->previous_active = ctx->active;
    ctx->active.owner = id;
    ctx->active.item = item;
    ctx->flags |= IMMCTX_ACTIVE_SET;
}
void hg_reset_active(HG_Context* ctx) {
    ctx->active.owner = -1;
    ctx->active.item = -1;
}

bool hg_active_item(HG_Context* ctx, s64 id, int item) {
    return (ctx->active.owner == id && ctx->active.item == item);
}

// Layout
static bool hg_layout_rectangle_top_down_single_column(HG_Context* ctx, vec2* position, r32 *width, r32* height, Clipping_Rect* clipping) {
    if(*height == 0.0f) {
        // default height is 30 px
        *height = 30.0f;
    } else if(*height <= 1.0f) {
        *height *= (ctx->current_frame.clipping.w);
    }

    if(*width == 0.0f) {
        *width = ctx->current_frame.clipping.z;
    } else if(*width <= 1.0f) {
        *width *= ctx->current_frame.clipping.z;
    }

    // Position in the current context
    position->x = ctx->current_frame.x;
    position->y = ctx->current_frame.y + ctx->current_frame.height - *height;

    if(ctx->current_frame_set) {
        ctx->current_frame.height -= (*height);
    }
    if (*width > ctx->current_frame.max_width) {
        ctx->current_frame.max_width = *width;
    }

    *clipping = ctx->current_frame.clipping;

    return true;
}

static void calculate_dimensions(r32* width, r32* height, r32 max_width, r32 max_height) {
    if(*height == 0.0f) {
        *height = max_height;
    } else if(*height <= 1.0f) {
        *height *= max_height;
    }

    if(*width == 0.0f) {
        *width = max_width;
    } else if(*width <= 1.0f) {
        *width *= max_width;
    }
}

// Layout
static bool hg_layout_rectangle_top_down_multiple_columns(HG_Context* ctx, vec2* position, r32 *width, r32* height, Clipping_Rect* clipping) {
    calculate_dimensions(width, height, ctx->current_frame.clipping.z, ctx->current_frame.clipping.w);
    r32 w = *width, h = *height;
    int index = ctx->current_frame.column_index;

    // Calculate width related positioning
    r32 col_width = ctx->current_frame.column_info[index].width;
    col_width = (col_width <= 1.0f) ? col_width * w : col_width;
    *width = col_width;

    r32 col_offset_x = 0.0f;
    for(s32 i = 0; i < index; ++i) {
        r32 cw = ctx->current_frame.column_info[i].width;
        col_offset_x += ((cw <= 1.0f) ? cw * w : cw);
    }

    // Position in the current context
    position->x = ctx->current_frame.x + col_offset_x;
    position->y = ctx->current_frame.y + ctx->current_frame.column_info[index].current_height - *height;

    if(ctx->current_frame_set) {
        ctx->current_frame.column_info[index].current_height -= (*height);
    }
    if (*width > ctx->current_frame.max_width) {
        ctx->current_frame.max_width = *width;
    }

    *clipping = ctx->current_frame.clipping;

    return true;
}

// Layout
bool hg_layout_rectangle_top_down(HG_Context* ctx, vec2* position, r32 *width, r32* height, Clipping_Rect* clipping) {
    if(ctx->current_frame.column_count > 1) {
        return hg_layout_rectangle_top_down_multiple_columns(ctx, position, width, height, clipping);
    } else {
        return hg_layout_rectangle_top_down_single_column(ctx, position, width, height, clipping);
    }
    return true;
}