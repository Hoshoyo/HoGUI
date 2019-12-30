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

bool hg_do_button(void* ctx, int id, const char* text) {
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

    u32 flags = 0;
    if(input_inside(input_mouse_position(), (vec4){0.0, 0.0, 100.0f, 100.0f})) {
        set_hot(ctx, id);
    }

    // Draw

    if(active(ctx, id)) {
        Quad_2D q = quad_new((vec2){0,0}, 100.0f, 100.0f, (vec4){1.0f, 0.3f, 0.3f, 1.0f});
        renderer_imm_quad(&q);
    } else if(hot(ctx, id)) {
        Quad_2D q = quad_new((vec2){0,0}, 100.0f, 100.0f, (vec4){1.0f, 0.4f, 0.4f, 1.0f});
        renderer_imm_quad(&q);
    } else {
        Quad_2D q = quad_new((vec2){0,0}, 100.0f, 100.0f, (vec4){0.7f, 0.3f, 0.3f, 1.0f});
        renderer_imm_quad(&q);
    }

    extern Font_Info font_info;
    //renderer_imm_debug_text(&font_info, (vec2){10,10}, (char*)text);
    Text_Render_Character_Position pos = {0};
    pos.index = 1;
    text_prerender(&font_info, "Hello", sizeof("Hello")-1, &pos, 1);

    return result;
}

void hg_end_frame(HG_Context* ctx) {
    if(!(ctx->flags & IMMCTX_HOT_SET))
        reset_hot(ctx);

    ctx->flags = 0;
}