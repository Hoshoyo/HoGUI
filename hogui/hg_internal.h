#ifdef __GNUC__
#define FUNCTION_IS_NOT_USED __attribute__ ((unused))
#else
#define FUNCTION_IS_NOT_USED
#endif

static const u32 IMMCTX_HOT_SET = (1 << 0);
static const u32 IMMCTX_ACTIVE_SET = (1 << 1);

static void FUNCTION_IS_NOT_USED hg_update(HG_Context* ctx) {
}

static bool FUNCTION_IS_NOT_USED active(HG_Context* ctx, s64 id) {
    return (ctx->active.owner == id);
}

static bool FUNCTION_IS_NOT_USED active_item(HG_Context* ctx, s64 id, int item) {
    return (ctx->active.owner == id && ctx->active.item == item);
}

static bool FUNCTION_IS_NOT_USED hot(HG_Context* ctx, s64 id) {
    return (ctx->hot.owner == id);
}

static bool FUNCTION_IS_NOT_USED hot_item(HG_Context* ctx, s64 id, int item) {
    return (ctx->hot.owner == id && ctx->hot.item == item);
}

static void FUNCTION_IS_NOT_USED set_hot(HG_Context* ctx, s64 id, int item) {
    ctx->last_hot.owner = id;
    ctx->last_hot.item = item;
    ctx->flags |= IMMCTX_HOT_SET;
}

static void FUNCTION_IS_NOT_USED reset_hot(HG_Context* ctx) {
    ctx->hot.owner = -1;
    ctx->hot.item = -1;
}

static vec4 FUNCTION_IS_NOT_USED color_from_hex(u32 value) {
    return (vec4){
        (r32)((value & 0xff0000) >> 16) / 255.0f,
        (r32)((value & 0xff00) >> 8) / 255.0f,
        (r32)((value & 0xff)) / 255.0f,
        1.0f
    };
}

static r32 FUNCTION_IS_NOT_USED lerp(r32 v0, r32 v1, r32 t) {
    return (1 - t) * v0 + t * v1;
}

extern Font_Info font_info;

bool hg_layout_rectangle_top_down(HG_Context* ctx, vec2* position, r32 *width, r32* height, Clipping_Rect* clipping);