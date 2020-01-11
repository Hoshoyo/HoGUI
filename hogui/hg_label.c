#include "immgui.h"
#include "../renderer/renderer_imm.h"
#include "immgui_input.h"
#include "hg_internal.h"
#include "colors.h"

static Clipping_Rect
label_render_auto_layout(HG_Context* ctx, int id, const char* text, int text_length, vec4 color) {
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
    if(hot(ctx, id)) {
        color_text = color_text_hot;
    }

    // Render label text, laying it out before in the center
    Text_Render_Info info = text_prerender(&font_info, text, text_length, 0, 0);
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
    
    text_render(&font_info, text, text_length, text_position, label_clipping, color_text);

    return label_clipping;
}

bool hg_do_label(HG_Context* ctx, int id, const char* text, int text_length, vec4 color) {
    hg_update(ctx);
    bool result = false;

    Clipping_Rect label_clipping = label_render_auto_layout(ctx, id, text, text_length, color);

    if(result = input_inside(input_mouse_position(), label_clipping)) {
        set_hot(ctx, id);
    }

    return result;
}