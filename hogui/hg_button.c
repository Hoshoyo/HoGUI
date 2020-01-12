#include "immgui.h"
#include "../renderer/renderer_imm.h"
#include "immgui_input.h"
#include "hg_internal.h"
#include "colors.h"

static Clipping_Rect
button_render_auto_layout(HG_Context* ctx, int id, const char* text, int text_length, bool enabled) {
    // Design parameters
    vec4          color = color_from_hex(pallete1_3);
    vec4          color_active = color_from_hex(pallete1_4);
    vec4          color_hot = gm_vec4_add(color_from_hex(pallete1_3), (vec4){0.05f, 0.05f, 0.1f, 0.0f});
    vec4          color_text = (vec4){0.2f, 0.2f, 0.24f, 1.0f};
    vec4          color_border[] = 
    {
        (vec4){0.3f, 0.3f, 0.3f, 1.0f}, 
        (vec4){0.5f, 0.5f, 0.5f, 1.0f}, 
        (vec4){0.3f, 0.3f, 0.3f, 1.0f}, 
        (vec4){0.5f, 0.5f, 0.5f, 1.0f}
    };

    if(!enabled) {
        color_text.a = 0.4f;
    }

    // Positioning parameters
    vec2          position = (vec2){0.0f, 0.0f};
    Clipping_Rect button_clipping = {0};
    r32           width = 0.0f;
    r32           height = 25.0f; // default height
    r32           border_width = 2.0f;
    
    hg_layout_rectangle_top_down(ctx, &position, &width, &height, &button_clipping);

    // Merge the clipping box of the input box with the context clipping
    button_clipping = clipping_rect_merge(button_clipping, (vec4){position.x, position.y, width, height});

    // Draw
    if(active(ctx, id)) {
        color = color_active;
    } else if(hot(ctx, id)) {
        color = color_hot;
    }

    // Render button quad
    Quad_2D button_quad = quad_new_clipped(position, width, height, color, button_clipping);
    renderer_imm_quad(&button_quad);

    // Render button border
    r32 button_border_width[] = {border_width, border_width, border_width, border_width};
    renderer_imm_border(&button_quad, button_border_width, color_border);

    // Render button text, laying it out before in the center
    Text_Render_Info info = text_prerender(&font_info, text, text_length, 0, 0);
    vec2 text_position = (vec2){position.x + (width - info.width) / 2.0f, position.y + (height - info.height) / 2.0f };
    
    text_render(&font_info, text, text_length, text_position, button_clipping, color_text);

    return button_clipping;
}

bool hg_do_button(HG_Context* ctx, int id, const char* text, int text_length, bool enabled) {
    hg_update(ctx);

    bool result = false;
    if(active(ctx, id)) {
        if(input_mouse_button_went_up(MOUSE_LEFT_BUTTON, 0, 0)) {
            if(hot(ctx, id)) result = true;
            reset_active(ctx);
        }
    } else if(hot(ctx, id)) {
        if(input_mouse_button_went_down(MOUSE_LEFT_BUTTON, 0, 0)) {
            set_active(ctx,id, 0);
        }
    }

    Clipping_Rect button_clipping = button_render_auto_layout(ctx, id, text, text_length, enabled);

    if(input_inside(input_mouse_position(), button_clipping)) {
        set_hot(ctx, id, 0);
    }

    return result;
}