#include "hhu.h"
#include "hhu_internal.h"
#include "batcher.h"

extern uint64_t asm_get_ret_addr();

void
hhu_window(float x, float y, float width, float height, hhu_color color)
{
    uint64_t id = asm_get_ret_addr();
    if(hhu_is_active(id, 0, 0))
    {

    }

    // render
    batch_render_quad_color_solid(&hhuctx->batch_ctx, (vec3){x, y, 0.0f}, width, height, *(vec4*)&color);
}