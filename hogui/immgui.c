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

bool hg_do_button(HG_Context* ctx, int id, const char* text, int text_length) {
    hg_update(ctx);
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

    // This is calculated automatically by the layout algorithm
    r32 width = 0.0f;
    r32 height = 25.0f;
    vec2 position = (vec2){0.0f, 0.0f};
    Clipping_Rect button_clipping = {0};
    hg_layout_rectangle_top_down(ctx, &position, &width, &height, &button_clipping);

    u32 flags = 0;
    if(input_inside(input_mouse_position(), (vec4){position.x, position.y, width, height})) {
        set_hot(ctx, id);
    }

    // Draw
    Quad_2D button_quad = {0};
    if(active(ctx, id)) {
        button_quad = quad_new(position, width, height, (vec4){1.0f, 0.3f, 0.3f, 1.0f});
    } else if(hot(ctx, id)) {
        button_quad = quad_new(position, width, height, (vec4){1.0f, 0.4f, 0.4f, 1.0f});
    } else {
        button_quad = quad_new(position, width, height, color_from_hex(pallete1_3));
    }
    Clipping_Rect clipping = clipping_rect_new_from_quad(&button_quad);
    renderer_imm_quad(&button_quad);

    r32 button_border_width[] = {2.0f, 2.0f, 2.0f, 2.0f};
    vec4 button_border_color[] = {(vec4){0.3f, 0.3f, 0.3f, 1.0f}, (vec4){0.5f, 0.5f, 0.5f, 1.0f}, (vec4){0.3f, 0.3f, 0.3f, 1.0f}, (vec4){0.5f, 0.5f, 0.5f, 1.0f}};
    renderer_imm_border(&button_quad, button_border_width, button_border_color);

    extern Font_Info font_info;

    int length_text = strlen(text);
    Text_Render_Info info = text_prerender(&font_info, text, text_length, 0, 0);
    vec2 text_position = (vec2){position.x + (width - info.width) / 2.0f, position.y + (height - info.height) / 2.0f };
    
    text_render(&font_info, text, text_length, text_position, clipping);

    return result;
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

void hg_window_begin(HG_Context* ctx, int id, vec2* in_position, r32 width, r32 height, const char* name, s32 vertical_column_count) {
    hg_update(ctx);
    vec2 position = *in_position;
    vec2 mouse_position = input_mouse_position();
    bool is_hot = hot(ctx, id);

    if(active(ctx, id)) {
        vec2 down_position = input_mouse_button_down_pos(MOUSE_LEFT_BUTTON);
        position.x += (mouse_position.x - down_position.x);
        position.y += (mouse_position.y - down_position.y);
        
        if(input_is_key_down(GLFW_KEY_LEFT_SHIFT))
            snap_position(&position, 30.0f);

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

    vec4 header_clipping = (vec4){position.x, position.y + height, width, 20.0f};
    if(input_inside(input_mouse_position(), header_clipping)) {
        set_hot(ctx, id);
    }

    Quad_2D q = quad_new(position, width, height, color_from_hex(pallete1_1));
    renderer_imm_quad(&q);

    r32 border_width[] = {1.0f, 1.0f, 0.0f, 1.0f};
    vec4 color_border = color_from_hex(pallete1_3);
    vec4 color[] = {color_border, color_border, color_border, color_border};
    renderer_imm_outside_border(&q, border_width, color);

    vec4 header_color = color_from_hex(pallete1_2);
    if(is_hot) {
        header_color.r -= 0.3f;
    }
    Quad_2D header = quad_new((vec2){position.x, position.y + height}, width, 20.0f, header_color);
    renderer_imm_quad(&header);
    r32 header_border_width[] = {1.0f, 1.0f, 1.0f, 0.0f};
    renderer_imm_outside_border(&header, header_border_width, color);

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