#include "hhu.h"
#include "hhu_internal.h"

#define MAX(A, B) (((A) > (B)) ? (A) : (B))
#define MIN(A, B) (((A) < (B)) ? (A) : (B))

static bool
is_inside(hhu_v2 point, float x, float y, float width, float height)
{
    return (
        (point.x >= x && point.x <= x + width) && 
        (point.y >= y && point.y <= y + height)
    );
}

static hhu_v2
mouse_pos()
{
    float x, y, w, h;
    hinp_window_size(&w, &h);
    hinp_mouse_position(&x, &y);
    return (hhu_v2){x, h - y};
}

static vec4
clipping_rect_merge(vec4 a, vec4 b) {
	vec4 result = {0};

	result.x = MAX(a.x, b.x);	// x
	result.y = MAX(a.y, b.y);	// y
	result.z = MIN(a.z - (result.x - a.x), b.z - (result.x - b.x));
	result.w = MIN(a.w - (result.y - a.y), b.w - (result.y - b.y));

	return result;
}

void
hhu_button_render(uint32_t id, int item, int index, const char* name, float button_size, vec3 position, float width, float size, vec4 clipping)
{
    vec4  button_color = batch_render_color_from_hex(0x121212ff);
    if(hhu_is_hot(id, item, index))
    {
        button_color = batch_render_color_from_hex(0xff0000ff);
    }

    batch_render_quad_color_solid_clipped(&hhuctx->batch_ctx, position, width, size, button_color, clipping);

    vec2 name_pos = (vec2){position.x, ceilf(position.y + (button_size / 2.0f) - (hhuctx->font_info.font_size / 2.0f))};
    text_render(&hhuctx->batch_ctx, &hhuctx->font_info, name, strlen(name), 0, name_pos, clipping, (vec4){1,1,1,1});
}

bool
hhu_button(uint32_t id, int item, int index, const char* name)
{
    float x, y, w, h;
    hhu_container_offset_get(&x, &y, &w, &h);

    vec4  clipping = (vec4){x, y, w, h};
    float button_size = 25.0f;
    vec3 button_position = (vec3){x, y + h - button_size, 0.0f};
    vec4 button_clipped = clipping_rect_merge(clipping, (vec4){button_position.x, button_position.y, w, button_size});
    bool inside_button = is_inside(mouse_pos(), button_clipped.x, button_clipped.y, button_clipped.z, button_clipped.w);

    bool result = false;

    hhu_container_offset_update(x, y, w, h - button_size);
    hhu_container_update_occupied_width(w);
    hhu_container_add_occupied_height(button_size);

    if(hhu_is_active(id, item, index))
    {
        hhu_v2 mp = mouse_pos();
        if(hinp_mouse_button_released(HINP_LBUTTON))
        {
            if(inside_button)
                result = true;
            hhu_set_active(0, 0, 0);
        }
    }

    hhu_update_hot_conditionally(inside_button, id, item, index);

    if(hhu_is_hot(id, item, index))
    {
        if(hinp_mouse_button_pressed(HINP_LBUTTON))
            hhu_set_active(id, item, index);
    }

    hhu_button_render(id, item, index, name, button_size, button_position, w, button_size, clipping);

    return result;
}