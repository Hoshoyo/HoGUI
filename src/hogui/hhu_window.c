#include "hhu.h"
#include "hhu_internal.h"
#include "batcher.h"

void
hhu_window(float x, float y, float width, float height, hhu_color color)
{
    batch_render_quad_color_solid(&hhuctx->batch_ctx, (vec3){x, y, 0.0f}, width, height, *(vec4*)&color);
}