#include "common.h"
#include "hhu.h"
#include "hhu_internal.h"
#include <stdlib.h>
#include "batcher.h"
#include "input.h"
#include "font_render.h"
#include "font_load.h"
#include <float.h>
#define GRAPHICS_MATH_IMPLEMENT
#include "gm.h"

static Font_Info font_info;

#if defined(_WIN32) || defined(_WIN64)
const char* font_filename = "C:/Windows/Fonts/consola.ttf";
#elif defined(__linux__)
const char* font_filename = "./res/LiberationMono-Regular.ttf";
#endif

HHU_Context*
hhu_init()
{
    HHU_Context_Internal* ctx = (HHU_Context_Internal*)calloc(1, sizeof(HHU_Context_Internal));
    batch_init(&ctx->batch_ctx);

    if(font_load(font_filename, &font_info, 22) != FONT_LOAD_OK)
    {
        fprintf(stderr, "Could not load font\n");
        return 0;
    }


    return (HHU_Context*)ctx;
}

int
hhu_destroy(HHU_Context* ctx)
{
    return 0;
}

void
hhu_render(HHU_Context* ctx)
{
    HHU_Context_Internal* ictx = (HHU_Context_Internal*)ctx;
    #if 1
    hinp_window_size(&ictx->batch_ctx.window_width, &ictx->batch_ctx.window_height);

    static float x = 0, y = 0;
    Hinp_Event ev = {0};
    while(hinp_event_next(&ev))
    {
        if(ev.type == HINP_EVENT_MOUSE_MOVE)
        {
            if(glfwGetMouseButton(ictx->window, 0) == 1)
            {
                if(hinp_query_mouse_inside())
                {
                    x += ev.mouse.deltax;
                    y += ev.mouse.deltay;
                }
            }
        }
    }
    batch_render_quad_color_solid(&ictx->batch_ctx, (vec3){x,ictx->batch_ctx.window_height - y,0}, 100, 100, (vec4){1,1,1,1});
    #endif

    //if(g_hhu_input.keyboard.keys['X'].state == 1)
    //{
    //    text_render(&ictx->batch_ctx, &font_info, "Hello", sizeof("Hello")-1, 0, (vec2){8.0,8.0}, (vec4){0,0,FLT_MAX,FLT_MAX}, (vec4){0,0,0,1});
    //}
    text_render(&ictx->batch_ctx, &font_info, "Hello#ff0000ff World", sizeof("Hello#ff0000ff World")-1, 0, (vec2){10,10}, (vec4){0,0,FLT_MAX,FLT_MAX}, (vec4){1,1,1,1});
    //text_render(&ictx->batch_ctx, &font_info, "Hello#ff0000ff World# fii", 5, 0, (vec2){10,10}, (vec4){0,0,FLT_MAX,FLT_MAX}, (vec4){1,1,1,1});

    batch_flush(&ictx->batch_ctx);
}

#if defined(HHU_USE_GLFW)
void
hhu_glfw_init(HHU_Context* ctx, GLFWwindow* window)
{
    HHU_Context_Internal* ictx = (HHU_Context_Internal*)ctx;
    ictx->window = window;

    hinp_init(window);
}
#endif