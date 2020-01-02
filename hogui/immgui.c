#include "immgui.h"
#include "../renderer/renderer_imm.h"
#include "immgui_input.h"

const u32 IMMCTX_HOT_SET = (1 << 0);
const u32 IMMCTX_ACTIVE_SET = (1 << 0);

static bool active(HG_Context* ctx, int id) {
    return (ctx->active.owner == id);
}

static bool hot(HG_Context* ctx, int id) {
    return (ctx->hot.owner == id);
}

static void reset_active(HG_Context* ctx) {
    ctx->active.owner = -1;
}

static void set_active(HG_Context* ctx, int id) {
    ctx->active.owner = id;
    ctx->flags |= IMMCTX_ACTIVE_SET;
}

static void set_hot(HG_Context* ctx, int id) {
    ctx->hot.owner = id;
    ctx->flags |= IMMCTX_HOT_SET;
}

static void reset_hot(HG_Context* ctx) {
    ctx->hot.owner = -1;
}

bool hg_do_button(HG_Context* ctx, int id, const char* text) {
    bool result = false;
    if(active(ctx, id)) {
        if(input_mouse_button_went_up(MOUSE_LEFT_BUTTON, 0, 0)) {
            if(hot(ctx, id)) result = true;
            reset_active(ctx);
        }
    } else if(hot(ctx, id)) {
        if(input_mouse_button_went_down(MOUSE_LEFT_BUTTON, 0, 0)) {
            set_active(ctx,id);
        }
    }

    r32 width = 100.0f;
    r32 height = 25.0f;
    vec2 position = (vec2){10.0f, 10.0f};

    u32 flags = 0;
    if(input_inside(input_mouse_position(), (vec4){position.x, position.y, width, height})) {
        set_hot(ctx, id);
    }

    // Draw
    if(active(ctx, id)) {
        Quad_2D q = quad_new(position, width, height, (vec4){1.0f, 0.3f, 0.3f, 1.0f});
        renderer_imm_quad(&q);
    } else if(hot(ctx, id)) {
        Quad_2D q = quad_new(position, width, height, (vec4){1.0f, 0.4f, 0.4f, 1.0f});
        renderer_imm_quad(&q);
    } else {
        Quad_2D q = quad_new(position, width, height, (vec4){0.7f, 0.3f, 0.3f, 1.0f});
        renderer_imm_quad(&q);
    }

    extern Font_Info font_info;
    renderer_imm_debug_text(&font_info, position, (char*)text);

    return result;
}

void hg_window_begin(HG_Context* ctx, int id, vec2 position, r32 width, r32 height, const char* name) {
    Quad_2D q = quad_new(position, width, height, (vec4){0.5f, 0.5f, 0.5f, 1.0f});
    renderer_imm_quad(&q);
}

void hg_end_frame(HG_Context* ctx) {
    if(!(ctx->flags & IMMCTX_HOT_SET))
        reset_hot(ctx);

    ctx->flags = 0;
}