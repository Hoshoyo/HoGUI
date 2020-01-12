static const u32 IMMCTX_HOT_SET = (1 << 0);
static const u32 IMMCTX_ACTIVE_SET = (1 << 1);

static void hg_update(HG_Context* ctx) {
}

static bool active(HG_Context* ctx, int id) {
    return (ctx->active.owner == id);
}

static bool active_item(HG_Context* ctx, int id, int item) {
    return (ctx->active.owner == id && ctx->active.item == item);
}

static bool hot(HG_Context* ctx, int id) {
    return (ctx->hot.owner == id);
}

static bool hot_item(HG_Context* ctx, int id, int item) {
    return (ctx->hot.owner == id && ctx->hot.item == item);
}

static void reset_active(HG_Context* ctx) {
    ctx->active.owner = -1;
    ctx->active.item = -1;
}

static void set_active(HG_Context* ctx, int id, int item) {
    ctx->previous_active = ctx->active;
    ctx->active.owner = id;
    ctx->active.item = item;
    ctx->flags |= IMMCTX_ACTIVE_SET;
}

static void set_hot(HG_Context* ctx, int id, int item) {
    ctx->last_hot.owner = id;
    ctx->last_hot.item = item;
    ctx->flags |= IMMCTX_HOT_SET;
}

static void reset_hot(HG_Context* ctx) {
    ctx->hot.owner = -1;
    ctx->hot.item = -1;
}

static vec4 color_from_hex(u32 value) {
    return (vec4){
        (r32)((value & 0xff0000) >> 16) / 255.0f,
        (r32)((value & 0xff00) >> 8) / 255.0f,
        (r32)((value & 0xff)) / 255.0f,
        1.0f
    };
}

static r32 lerp(r32 v0, r32 v1, r32 t) {
    return (1 - t) * v0 + t * v1;
}

extern Font_Info font_info;

bool hg_layout_rectangle_top_down(HG_Context* ctx, vec2* position, r32 *width, r32* height, Clipping_Rect* clipping);