#include "hhu.h"
#include "hhu_internal.h"
#include "batcher.h"

#define WINDOW_ID(X) ((uint32_t)(uint64_t)(X))

static bool
is_inside(hhu_v2 point, float x, float y, float width, float height)
{
    return (
        (point.x >= x && point.x <= x + width) && 
        (point.y >= y && point.y <= y + height)
    );
}

static bool
is_inside_header(hhu_v2 point, HGui_Window* w)
{
    return is_inside(point, w->x, w->y + w->height, w->width, w->header_size_px);
}

static bool
is_inside_user_area(hhu_v2 point, HGui_Window* w)
{
    return is_inside(point, w->x, w->y, w->width, w->height);
}

static bool
is_inside_window(hhu_v2 point, HGui_Window* w)
{
    return is_inside(point, w->x - w->border_size_px, w->y - w->border_size_px, 
        w->width + (w->border_size_px * 2), w->height + (w->border_size_px * 2) + w->header_size_px);
}

static bool
is_inside_border_left(hhu_v2 point, HGui_Window* w)
{
    return (
        (point.x >= (w->x - w->border_size_px)) &&
        (point.x <= w->x) &&
        (point.y >= w->y) &&
        (point.y <= (w->y + w->height + w->header_size_px))
    );
}

static bool
is_inside_border_right(hhu_v2 point, HGui_Window* w)
{
    return (
        (point.x >= (w->x + w->width)) &&
        (point.x <= (w->x + w->width + w->border_size_px)) &&
        (point.y >= w->y) &&
        (point.y <= (w->y + w->height + w->header_size_px))
    );
}

static bool
is_inside_border_bot(hhu_v2 point, HGui_Window* w)
{
    return (
        (point.x >= w->x) &&
        (point.x <= (w->x + w->width)) &&
        (point.y <= w->y) &&
        (point.y >= (w->y - w->border_size_px))
    );
}

static bool
is_inside_border_top(hhu_v2 point, HGui_Window* w)
{
    return (
        (point.x >= w->x) &&
        (point.x <= (w->x + w->width)) &&
        (point.y <= (w->y + w->height + w->border_size_px + w->header_size_px)) &&
        (point.y >= (w->y + w->height + w->header_size_px))
    );
}

static bool
is_inside_border(hhu_v2 point, HGui_Window* w)
{
    return (is_inside_border_left(point, w) && 
        is_inside_border_right(point, w) && 
        is_inside_border_top(point, w) && 
        is_inside_border_bot(point, w)
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

void
hhu_window_init_default(HGui_Window* window)
{
    window->flags |= HGUI_WINDOW_FLAG_INITIALIZED;
    window->width  = 320.0f;
    window->height = 240.0f;
    window->x = 100.0f;
    window->y = 100.0f;
    window->header_size_px = 26.0f;
    window->border_size_px = 10.0f;
}

void
hhu_window_render(HGui_Window* window)
{
    uint32_t id = WINDOW_ID(window);
    vec4 colors[4] = {
        batch_render_color_from_hex(0x505050ff),
        batch_render_color_from_hex(0x505050ff),
        batch_render_color_from_hex(0x101010ff),
        batch_render_color_from_hex(0x101010ff),
    };
    vec4 header_color = batch_render_color_from_hex(0x000000ff);
    vec4 border_color = batch_render_color_from_hex(0x050505ff);

    //if(hhu_is_hot(id, 0, 0)) {}
    
    if(hhu_is_active(id, 0, 0))
        border_color = batch_render_color_from_hex(0x0F0F0Fff);

    // body
    batch_render_quad_color(&hhuctx->batch_ctx, (vec3){window->x, window->y, 0.0f}, window->width, window->height, colors);

    float bsize = window->border_size_px;
    float hsize = window->header_size_px;

    // header
    {
        vec3 hpos = (vec3){window->x, window->y + window->height, 0.0f};
        batch_render_quad_color_solid(&hhuctx->batch_ctx, hpos, window->width, window->header_size_px, header_color);
    }

    hhu_v2 mp = mouse_pos();
    // border
    {
        // left
        bool inside_left = is_inside_border_left(mp, window);
        vec3 lpos[4] = {
            (vec3){window->x - bsize, window->y - bsize,                          0.0f},  // bl
            (vec3){window->x,         window->y,                                  0.0f},  // br
            (vec3){window->x - bsize, window->y + window->height + bsize + hsize, 0.0f},  // tl
            (vec3){window->x,         window->y + window->height + hsize,         0.0f},  // tr
        };
        batch_render_quad_free_color_solid(&hhuctx->batch_ctx, lpos, (inside_left) ? (vec4){1,0,0,1}: border_color);

        // right
        bool inside_right = is_inside_border_right(mp, window);
        vec3 rpos[4] = {
            (vec3){window->x + window->width,         window->y,                                  0.0f},  // bl
            (vec3){window->x + window->width + bsize, window->y - bsize,                          0.0f},  // br
            (vec3){window->x + window->width,         window->y + window->height + hsize,         0.0f},  // tl
            (vec3){window->x + window->width + bsize, window->y + window->height + bsize + hsize, 0.0f},  // tr
        };
        batch_render_quad_free_color_solid(&hhuctx->batch_ctx, rpos, (inside_right) ? (vec4){1,0,0,1}: border_color);

        // bottom
        bool inside_bot = is_inside_border_bot(mp, window);
        vec3 bpos[4] = {
            (vec3){window->x - bsize,                 window->y - bsize, 0.0f},  // bl
            (vec3){window->x + bsize + window->width, window->y - bsize, 0.0f},  // br
            (vec3){window->x,                         window->y,         0.0f},  // tl
            (vec3){window->x + window->width,         window->y,         0.0f},  // tr
        };
        batch_render_quad_free_color_solid(&hhuctx->batch_ctx, bpos, (inside_bot) ? (vec4){1,0,0,1}: border_color);

        // top
        bool inside_top = is_inside_border_top(mp, window);
        vec3 tpos[4] = {
            (vec3){window->x,                         window->y + window->height + hsize,         0.0f},  // bl
            (vec3){window->x + window->width,         window->y + window->height + hsize,         0.0f},  // br
            (vec3){window->x - bsize,                 window->y + window->height + bsize + hsize, 0.0f},  // tl
            (vec3){window->x + bsize + window->width, window->y + window->height + bsize + hsize, 0.0f},  // tr
        };
        batch_render_quad_free_color_solid(&hhuctx->batch_ctx, tpos, (inside_top) ? (vec4){1,0,0,1}: border_color);

    }
}

void
hhu_window(HGui_Window* window)
{
    uint32_t id = WINDOW_ID(window);
    if(!(window->flags & HGUI_WINDOW_FLAG_INITIALIZED))
        hhu_window_init_default(window);

    if(hhu_is_active(id, 0, 0))
    {
        if(hinp_mouse_button_down(HINP_LBUTTON))
        {
            float mdx, mdy;
            hinp_mouse_delta(&mdx, &mdy);
            hhu_v2 pos_now = mouse_pos();
            hhu_v2 pos_last_frame = (hhu_v2){pos_now.x - mdx, pos_now.y + mdy};

            if(is_inside_header(pos_last_frame, window))
            {
                window->x += mdx;
                window->y -= mdy;
            }
            if(is_inside_border_left(pos_last_frame, window))
            {
                window->x += mdx;
                window->width -= mdx;
                printf("%f\n", window->width);
            }
            if(is_inside_border_right(pos_last_frame, window))
            {
                window->width += mdx;
            }
        }
    }
    if(hhu_is_hot(id, 0, 0))
    {
        if(hinp_mouse_button_pressed(HINP_LBUTTON))
            hhu_set_active(id, 0, 0);
    }

    hhu_update_hot_conditionally(is_inside_window(mouse_pos(), window), id, 0, 0);

    hhu_window_render(window);
}