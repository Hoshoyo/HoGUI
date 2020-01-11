static const u32 IMMCTX_HOT_SET = (1 << 0);
static const u32 IMMCTX_ACTIVE_SET = (1 << 0);

static void hg_update(HG_Context* ctx) {
}

static bool active(HG_Context* ctx, int id) {
    return (ctx->active.owner == id);
}

static bool hot(HG_Context* ctx, int id) {
    return (ctx->hot.owner == id);
}

static void reset_active(HG_Context* ctx) {
    ctx->active.owner = -1;
}

static void set_active(HG_Context* ctx, int id) {
    ctx->previous_active = ctx->active;
    ctx->active.owner = id;
    ctx->flags |= IMMCTX_ACTIVE_SET;
}

static void set_hot(HG_Context* ctx, int id) {
    ctx->last_hot = id;
    ctx->flags |= IMMCTX_HOT_SET;
}

static void reset_hot(HG_Context* ctx) {
    ctx->hot.owner = -1;
}

static vec4 color_from_hex(u32 value) {
    return (vec4){
        (r32)((value & 0xff0000) >> 16) / 255.0f,
        (r32)((value & 0xff00) >> 8) / 255.0f,
        (r32)((value & 0xff)) / 255.0f,
        1.0f
    };
}

extern Font_Info font_info;