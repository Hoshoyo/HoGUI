#include "hg_ui.h"
#include "../renderer/renderer_imm.h"
#include "hg_input.h"
#include "hg_internal.h"
#include "colors.h"
#include <float.h>

bool hg_do_graph(HG_Context* ctx, s64 id, int item, r32* values, s32 count, r32 initial_max, vec4 color) {
    r32 width = 1.0f, height = 1.0f;
    vec2 position = {0};
    Clipping_Rect clipping = {0};

    hg_layout_rectangle_top_down(ctx, &position, &width, &height, &clipping);

    r32 single_width = ceil(width / (r32)count);

    r32 max_value = initial_max;
    r32 min_value = FLT_MAX;
    s32 max_value_index = 0;
    s32 min_value_index = 0;
    for(s32 i = 0; i < count; ++i) {
        if(values[i] > max_value) { max_value = values[i]; max_value_index = i; }
        if(values[i] < min_value) { min_value = values[i]; min_value_index = i; }
    }

    for(s32 i = 0; i < count; ++i) {
        r32 normalized = values[i] / max_value;
        r32 h = normalized * height;

        vec4 cc = color;
        if(normalized >= 0.1f) cc = (vec4){1,0,0,1};

        Quad_2D q = quad_new_clipped(position, single_width, h, cc, clipping);
        renderer_imm_quad(&q);

        position.x += single_width;
    }

    return true;
}