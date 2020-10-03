#include "common.h"
#include "hhu.h"
#include <stdint.h>

typedef struct {
    float xoff, yoff;
    float width, height;

    hhu_v4 clip_rect;
} Gui_Layout_Node;

typedef struct {
    uint64_t id;
    int      item;
    int      index;
} Gui_Element;

typedef struct {
    Gui_Element hot;
    Gui_Element active;
} Gui_Ctx;

static Gui_Ctx gui_ctx;

bool
hhu_is_active(uint32_t id, int item, int index)
{
    return gui_ctx.active.id == id && gui_ctx.active.item == item && gui_ctx.active.index;
}

bool
hhu_is_hot(uint32_t id, int item, int index)
{
    return gui_ctx.hot.id == id && gui_ctx.hot.item == item && gui_ctx.hot.index;
}