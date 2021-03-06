#include "hg_ui.h"
#include "../renderer/renderer_imm.h"
#include "hg_input.h"
#include "hg_internal.h"
#include "colors.h"

static Clipping_Rect
slider_render_auto_layout(HG_Context* ctx, s64 id, int item, r32 value, r32 min, r32 max) {
    // Design parameters
    vec4 color_base = color_from_hex(pallete1_2);
    vec4 color_handle = color_from_hex(pallete1_3);
    vec4 color_handle_border = color_from_hex(pallete1_1);

    // Positioning parameters
    vec2          position = {0};
    Clipping_Rect slider_clipping = {0};
    r32           width = 0.0f;
    r32           height = 25.0f;
    r32           h_margin = 4.0f;
    r32           v_margin = 2.0f;
    r32           base_size = 2.0f;
    r32           handle_width = 8.0f;
    r32           handle_border_width = 1.0f;

    hg_layout_rectangle_top_down(ctx, &position, &width, &height, &slider_clipping);

    // Merge the clipping box of the input box with the context clipping
    slider_clipping = clipping_rect_merge(slider_clipping, (vec4){position.x, position.y, width, height});

    vec2 base_position = (vec2) {position.x + h_margin, position.y + ((height / 2.0f) - (base_size / 2.0f))};
    r32  base_width = width - h_margin * 2.0f;

    // Render base
    Quad_2D base = quad_new_clipped(base_position, base_width, base_size, color_base, slider_clipping);
    renderer_imm_quad(&base);

    // Calculate layout within rect
    r32 range = max - min;
    r32 percentage = (value - min) / range;

    vec2 handle_position = (vec2){
        lerp(position.x + h_margin, position.x + width - h_margin, percentage) - (handle_width / 2.0f), 
        position.y + v_margin
    };
    r32  handle_height = height - 2.0f * v_margin;

    // Render handle
    Quad_2D handle = quad_new_clipped(handle_position, handle_width, handle_height, color_handle, slider_clipping);
    renderer_imm_quad(&handle);

    // Render handle border
    renderer_imm_border_clipped_simple(handle, handle_border_width, color_handle_border, slider_clipping);

    return slider_clipping;
}

void 
hg_do_slider(HG_Context* ctx, s64 id, int item, r32* value, r32 min, r32 max) {
    hg_update(ctx);

    vec2 mouse_position = input_mouse_position();

    if(active_item(ctx, id, item)) {
        if(input_mouse_button_went_up(MOUSE_LEFT_BUTTON, 0, 0)) {
            hg_reset_active(ctx);
        }
    } else if(hot_item(ctx, id, item)) {
        if(input_mouse_button_went_down(MOUSE_LEFT_BUTTON, 0, 0)) {
            hg_set_active(ctx, id, item);
        }
    }
    
    Clipping_Rect slider_clipping = slider_render_auto_layout(ctx, id, item, *value, min, max);
    
    if(active_item(ctx, id, item)) {
        r32 center = slider_clipping.x + slider_clipping.z / 2.0f;
        r32 diff = mouse_position.x - center;
        r32 w = slider_clipping.z;
        r32 percent = MIN(MAX(0.5f + (diff / w), 0.0f), 1.0f);
        
        *value = lerp(min, max, percent);
    }

    //renderer_imm_debug_box(slider_clipping.x, slider_clipping.y, slider_clipping.z, slider_clipping.w, (vec4){1.0f, 1.0f, 0.0f, 1.0f});

    if(input_inside(input_mouse_position(), slider_clipping)) {
        set_hot(ctx, id, item);
    }
}