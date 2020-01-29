#include "hg_ui.h"
#include "../renderer/renderer_imm.h"
#include "hg_input.h"
#include "hg_internal.h"
#include "colors.h"

static Clipping_Rect
label_render_auto_layout(HG_Context* ctx, s64 id, int item, const char* text, int text_length, vec4 color) {
    // Design parameters
    vec4          color_text = color;
    vec4          color_text_hot = gm_vec4_subtract(color, (vec4){0.1f, 0.1f, 0.1f, 0.0f});

    // Positioning parameters
    vec2          position = (vec2){0.0f, 0.0f};
    Clipping_Rect label_clipping = {0};
    r32           width = 0.0f;
    r32           height = 25.0f; // default height
    int           align = -1; // -1 left, 0 center, 1 right
    r32           margin = 4.0f;
    
    hg_layout_rectangle_top_down(ctx, &position, &width, &height, &label_clipping);

    // Merge the clipping box of the input box with the context clipping
    label_clipping = clipping_rect_merge(label_clipping, (vec4){position.x, position.y, width, height});

    // Draw
    if(hot_item(ctx, id, item)) {
        color_text = color_text_hot;
    }

    // Render label text, laying it out before in the center
    Text_Render_Info info = text_prerender(&ctx->font_info, text, text_length, 0, 0, 0);
    vec2 text_position = {0};
    switch(align) {
        case -1: {
            text_position = (vec2){position.x + margin, position.y + (height - info.height) / 2.0f };
        } break;
        case 1: {
            text_position = (vec2){position.x + width - info.width - margin, position.y + (height - info.height) / 2.0f };
        } break;
        case 0: {
            text_position = (vec2){position.x + (width - info.width) / 2.0f, position.y + (height - info.height) / 2.0f };
        }
        default: break;
    }
    
    text_render(&ctx->font_info, text, text_length, 0, text_position, label_clipping, color_text);

    //renderer_imm_debug_box(label_clipping.x, label_clipping.y, label_clipping.z, label_clipping.w, (vec4){1.0f, 0.0f, 0.0f, 1.0f});

    return label_clipping;
}

r32 hg_do_label_slider(HG_Context* ctx, s64 id, int item, const char* text, int text_length, vec4 color) {
    hg_update(ctx);
    r32 result = 0.0f;

    vec2 mouse_position = input_mouse_position();
    vec2 mouse_last_position = input_mouse_last_position();

    if(active_item(ctx, id, item)) {
        if(input_mouse_button_went_up(MOUSE_LEFT_BUTTON, 0, 0)) {
            hg_reset_active(ctx);
        } else {
            r32 diff = mouse_position.x - mouse_last_position.x;
            result = diff;
        }
    } else if(hot_item(ctx, id, item)) {
        if(input_mouse_button_went_down(MOUSE_LEFT_BUTTON, 0, 0)) {
            hg_set_active(ctx, id, item);
        }
    }

    Clipping_Rect label_clipping = label_render_auto_layout(ctx, id, item, text, text_length, color);

    if(input_inside(input_mouse_position(), label_clipping)) {
        set_hot(ctx, id, item);
    }

    return result;
}