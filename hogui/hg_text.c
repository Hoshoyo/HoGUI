#include "hg_ui.h"
#include "../renderer/renderer_imm.h"
#include "hg_input.h"
#include "hg_internal.h"
#include "colors.h"

static Clipping_Rect
text_render_auto_layout(HG_Context* ctx, s64 id, int item, const char* text, int text_length, int start_index, vec4 color) {
    // Design parameters
    vec4          color_text = color;
    vec4          color_text_hot = gm_vec4_subtract(color, (vec4){0.1f, 0.1f, 0.1f, 0.0f});

    // Positioning parameters
    vec2          position = (vec2){0.0f, 0.0f};
    Clipping_Rect label_clipping = {0};
    r32           width = 0.0f;
    r32           height = 0.0f; // default height
    int           align = -1; // -1 left, 0 center, 1 right
    r32           margin = 4.0f;
    
    // Render label text, laying it out before in the center
    Text_Render_Info info = text_prerender(&ctx->font_info, text, text_length, start_index, 0, 0);
    height = info.height + ctx->font_info.max_height;
    width = info.width;
    
    hg_layout_rectangle_top_down(ctx, &position, &width, &height, &label_clipping);

    //renderer_imm_debug_box(position.x, position.y, width, height, (vec4){1.0f, 0.0f, 0.0f, 1.0f});

    // Merge the clipping box of the input box with the context clipping
    label_clipping = clipping_rect_merge(label_clipping, (vec4){position.x, position.y, width, height});

    // Draw
    if(hot_item(ctx, id, item)) {
        color_text = color_text_hot;
    }

    vec2 text_position = {0};
    switch(align) {
        case -1: {
            text_position = (vec2){position.x + margin, position.y + height - ctx->font_info.max_height};
        } break;
        case 1: {
            text_position = (vec2){position.x + width - info.width - margin, position.y + height - ctx->font_info.max_height };
        } break;
        case 0: {
            text_position = (vec2){position.x + (width - info.width) / 2.0f, position.y + height - ctx->font_info.max_height  };
        }
        default: break;
    }
    
    text_render(&ctx->font_info, text, text_length, start_index, text_position, label_clipping, color_text);

    //renderer_imm_debug_box(label_clipping.x, label_clipping.y, label_clipping.z, label_clipping.w, (vec4){1.0f, 0.0f, 0.0f, 1.0f});

    return label_clipping;
}

bool hg_do_text(HG_Context* ctx, s64 id, int item, const char* text, int text_length, vec4 color) {
    Clipping_Rect clipping = {0};
    return hg_do_text_advanced(ctx, id, item, text, text_length, 0, color, &clipping);
}

bool hg_do_text_advanced(HG_Context* ctx, s64 id, int item, const char* text, int text_length, int start_index, vec4 color, Clipping_Rect* out_bbox) {
    hg_update(ctx);
    bool result = false;

    Clipping_Rect label_clipping = text_render_auto_layout(ctx, id, item, text, text_length, start_index, color);

    if(result = input_inside(input_mouse_position(), label_clipping)) {
        set_hot(ctx, id, item);
    }

    return result;
}