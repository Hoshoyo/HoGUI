
#include "common.h"
#include "hhu.h"
#include "hhu_internal.h"
#include <stdlib.h>
#include "input.h"
#include "font_render.h"
#include "font_load.h"
#include <float.h>
#define GRAPHICS_MATH_IMPLEMENT
#include "gm.h"
#include "batcher.h"
#include "hhu_internal.h"

hhu_color hhu_color_red   = {1,0,0,1};
hhu_color hhu_color_green = {0,1,0,1};
hhu_color hhu_color_blue  = {0,0,1,1};

#if defined(_WIN32) || defined(_WIN64)
const char* font_filename = "C:/Windows/Fonts/consola.ttf";
#elif defined(__linux__)
const char* font_filename = "./res/LiberationMono-Regular.ttf";
#endif

HHU_Context* hhuctx;

int
hhu_init()
{
    hhuctx = (HHU_Context*)calloc(1, sizeof(HHU_Context));
    batch_init(&hhuctx->batch_ctx);

    if(font_load(font_filename, &hhuctx->font_info, 14) != FONT_LOAD_OK)
    {
        fprintf(stderr, "Could not load font\n");
        return -1;
    }

    hhu_internal_init();
    return 0;
}

int
hhu_destroy()
{
    free(hhuctx);
    return 0;
}

void
hhu_begin()
{
    hinp_window_size(&hhuctx->batch_ctx.window_width, &hhuctx->batch_ctx.window_height);
    if(hinp_mouse_button_pressed(HINP_LBUTTON) && hhu_is_hot(0,0,0))
    {
        hhu_set_active(0,0,0);
    }
}

void
hhu_end()
{
    hhu_internal_end();
    hhu_reset_hot_index();
    hinp_clear();
    batch_flush(&hhuctx->batch_ctx);
}

#if defined(HHU_USE_GLFW)
void
hhu_glfw_init(GLFWwindow* window)
{
    hhuctx->window = window;

    hinp_init(window);
}
#endif