#include "common.h"
#include "hhu.h"
#include <stdint.h>
#include "light_array.h"

typedef struct {
    uint64_t id;
    int      item;
    int      index;
} Gui_Element;

typedef struct {
    float x, y;
    float width, height;
} Gui_Container;

typedef struct {
    Gui_Element hot;
    int         hot_index;
    int         last_hot_index;
    Gui_Element active;

    // Container stack
    Gui_Container* cont_stack;
} Gui_Ctx;

static Gui_Ctx gui_ctx;

bool
hhu_is_active(uint32_t id, int item, int index)
{
    return gui_ctx.active.id == id && gui_ctx.active.item == item && gui_ctx.active.index == index;
}

bool
hhu_is_hot(uint32_t id, int item, int index)
{
    return gui_ctx.hot.id == id && gui_ctx.hot.item == item && gui_ctx.hot.index == index;
}

void
hhu_set_active(uint32_t id, int item, int index)
{
    gui_ctx.active.id = id;
    gui_ctx.active.item = item;
    gui_ctx.active.index = index;
}

void
hhu_reset_hot_index()
{
    gui_ctx.hot_index = 0;
}

void
hhu_set_hot(uint32_t id, int item, int index)
{
    if(id == 0 || (gui_ctx.hot_index >= gui_ctx.last_hot_index))
    {
        gui_ctx.hot.id = id;
        gui_ctx.hot.item = item;
        gui_ctx.hot.index = index;
        gui_ctx.last_hot_index = gui_ctx.hot_index;
        gui_ctx.hot_index++;
    }
}

void
hhu_update_hot_conditionally(bool condition, uint32_t id, int item, int index)
{
    if(condition)
    {
        hhu_set_hot(id, item, index);
    }
    else if(hhu_is_hot(id, item, index))
    {
        hhu_set_hot(0, 0, 0);
    }
}

void
hhu_internal_init()
{
    gui_ctx.cont_stack = array_new(Gui_Container);
}

void
hhu_push_container_stack(float x, float y, float width, float height)
{
    Gui_Container cont = {0};
    cont.x = x;
    cont.y = y;
    cont.width = width;
    cont.height = height;
    array_push(gui_ctx.cont_stack, cont);
}

void
hhu_internal_end()
{
    array_clear(gui_ctx.cont_stack);
}