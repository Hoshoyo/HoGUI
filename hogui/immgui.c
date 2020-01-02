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

    Text_Render_Character_Position char_pos = {0};
    char_pos.index = 1;
    Text_Render_Info info = text_prerender(&font_info, "Hello", sizeof("Hello")-1, &char_pos, 1);
    r32 hey = info.bounding_box.w - info.bounding_box.y;
    r32 wid = info.bounding_box.z - info.bounding_box.x;

    vec2 text_position = (vec2){position.x + (width - wid) / 2.0f, position.y + (height - hey) / 2.0f };
    
    text_render(&font_info, "Hello", sizeof("Hello")-1, text_position);
    renderer_imm_debug_box(
        char_pos.position.x + text_position.x + 1.0f, 
        text_position.y - 3.0f, 
        font_info.max_width, 
        font_info.max_height, 
        (vec4){1.0f, 1.0f, 1.0f, 1.0f});

    Quad_2D q = quad_new((vec2){char_pos.position.x + text_position.x + 1.0f, text_position.y - 3.0f}, font_info.max_width, font_info.max_height, (vec4){1.0f, 1.0f, 1.0f, 0.3f});
    renderer_imm_quad(&q);

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