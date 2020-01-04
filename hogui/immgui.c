#include "immgui.h"
#include "../renderer/renderer_imm.h"
#include "immgui_input.h"

const u32 IMMCTX_HOT_SET = (1 << 0);
const u32 IMMCTX_ACTIVE_SET = (1 << 0);

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
    ctx->active.owner = id;
    ctx->flags |= IMMCTX_ACTIVE_SET;
}

static void set_hot(HG_Context* ctx, int id) {
    ctx->hot.owner = id;
    ctx->flags |= IMMCTX_HOT_SET;
}

static void reset_hot(HG_Context* ctx) {
    ctx->hot.owner = -1;
}

void hg_end_frame(HG_Context* ctx) {
    if(!(ctx->flags & IMMCTX_HOT_SET))
        reset_hot(ctx);

    ctx->flags = 0;
}

bool hg_do_button(HG_Context* ctx, int id, const char* text) {
    bool result = false;
    if(active(ctx, id)) {
        if(input_mouse_button_went_up(MOUSE_LEFT_BUTTON, 0, 0)) {
            if(hot(ctx, id)) result = true;
            reset_active(ctx);
        }
    } else if(hot(ctx, id)) {
        if(input_mouse_button_went_down(MOUSE_LEFT_BUTTON, 0, 0)) {
            set_active(ctx,id);
        }
    }

    r32 width = 100.0f;
    r32 height = 25.0f;
    vec2 position = (vec2){10.0f, 10.0f};

    u32 flags = 0;
    if(input_inside(input_mouse_position(), (vec4){position.x, position.y, width, height})) {
        set_hot(ctx, id);
    }

    // Draw
    if(active(ctx, id)) {
        Quad_2D q = quad_new(position, width, height, (vec4){1.0f, 0.3f, 0.3f, 1.0f});
        renderer_imm_quad(&q);
    } else if(hot(ctx, id)) {
        Quad_2D q = quad_new(position, width, height, (vec4){1.0f, 0.4f, 0.4f, 1.0f});
        renderer_imm_quad(&q);
    } else {
        Quad_2D q = quad_new(position, width, height, (vec4){0.7f, 0.3f, 0.3f, 1.0f});
        renderer_imm_quad(&q);
    }

    extern Font_Info font_info;

    Text_Render_Character_Position char_pos = {0};
    char_pos.index = 1;
    Text_Render_Info info = text_prerender(&font_info, "Hello", sizeof("Hello")-1, &char_pos, 1);
    vec2 text_position = (vec2){position.x + (width - info.width) / 2.0f, position.y + (height - info.height) / 2.0f };
    
    text_render(&font_info, "Hello", sizeof("Hello")-1, text_position, font_render_no_clipping());
    renderer_imm_debug_box(
        char_pos.position.x + text_position.x + 1.0f, 
        text_position.y - 3.0f, 
        font_info.max_width, 
        font_info.max_height, 
        (vec4){1.0f, 1.0f, 1.0f, 1.0f});

    Quad_2D q = quad_new((vec2){char_pos.position.x + text_position.x + 1.0f, text_position.y - 3.0f}, font_info.max_width, font_info.max_height, (vec4){1.0f, 1.0f, 1.0f, 0.3f});
    renderer_imm_quad(&q);

    return result;
}

static int length_current_utf8(char* buffer) {
    int l = 1;
    while (* buffer && l < 4 && ((*buffer) & 0xc0) == 0x80) {
        buffer--;
        l++;
    }
    return l;
}

static int length_next_utf8(char* buffer, int max) {
    int l = 1;
    buffer++; // skip first
    while (*buffer && l < max && ((*buffer) & 0xc0) == 0x80) {
        buffer++;
        l++;
    }
    return l;
}

static int delete_selection_forward(char* buffer, int* cursor_index, int* selection_distance, int* buffer_length) {
    if(*selection_distance > 0) {
        memcpy(buffer + *cursor_index, buffer + *cursor_index + *selection_distance, *buffer_length - (*cursor_index + *selection_distance));
        *buffer_length -= (*selection_distance);
        *selection_distance = 0;
    } else if(*selection_distance < 0) {
        memcpy(buffer + *cursor_index + *selection_distance, buffer + *cursor_index, *buffer_length - *cursor_index);
        *buffer_length += (*selection_distance);
        *cursor_index += (*selection_distance);
        *selection_distance = 0;
    } else {
        if(*cursor_index < *buffer_length) {
            int l = length_next_utf8(buffer + *cursor_index, *buffer_length - *cursor_index);
            memcpy(buffer + *cursor_index, buffer + *cursor_index + l, *buffer_length - *cursor_index - l);
            *buffer_length -= l;
            return l;
        }
    }
    return 0;
}
static int delete_selection_backward(char* buffer, int* cursor_index, int* selection_distance, int* buffer_length) {
    if(*selection_distance > 0) {
        memcpy(buffer + *cursor_index, buffer + *cursor_index + *selection_distance, *buffer_length - (*cursor_index + *selection_distance));
        *buffer_length -= (*selection_distance);
        *selection_distance = 0;
    } else if(*selection_distance < 0) {
        memcpy(buffer + *cursor_index + *selection_distance, buffer + *cursor_index, *buffer_length - *cursor_index);
        *buffer_length += (*selection_distance);
        *cursor_index += (*selection_distance);
        *selection_distance = 0;
    } else {
        if (*buffer_length > 0 && *cursor_index > 0) {
            int l = length_current_utf8(buffer + *cursor_index - 1);
            memcpy(buffer + *cursor_index - l, buffer + *cursor_index, *buffer_length - *cursor_index);
            (*buffer_length) -= l;
            (*cursor_index) -= l;
        }
    }
    return 0;
}

// selection_distance == 0 means no selection is active
bool hg_do_input(HG_Context* ctx, int id, char* buffer, int buffer_max_length, int* buffer_length, int* cursor_index, int* selection_distance) {
    bool result = true;
    if(active(ctx, id)) {
        u32 key = 0;
        s32 mods = 0;
        char utf8_key[4] = {0};
        while(input_next_key_pressed(&key, &mods)) {
            
            switch(key) {
                case GLFW_KEY_END:{
                    if(mods & GLFW_MOD_SHIFT) {
                        *selection_distance = -(*buffer_length - *cursor_index - *selection_distance);
                    }
                    *cursor_index = *buffer_length;
                } break;
                case GLFW_KEY_HOME: {
                    if(mods & GLFW_MOD_SHIFT) {
                        *selection_distance = *cursor_index + *selection_distance;
                    }
                    *cursor_index = 0;
                } break;
                case GLFW_KEY_LEFT: {
                    if(*cursor_index > 0) {
                        int l = length_current_utf8(buffer + *cursor_index - 1);
                        (*cursor_index) -= l;
                        if(mods & GLFW_MOD_SHIFT) {
                            *selection_distance += l;
                        }
                    }
                } break;
                case GLFW_KEY_RIGHT: {
                    if(*cursor_index < *buffer_length) {
                        int l = length_next_utf8(buffer + *cursor_index, *buffer_length - *cursor_index);
                        (*cursor_index) += l;
                        if(mods & GLFW_MOD_SHIFT) {
                            *selection_distance -= 1;
                        }
                    }
                } break;
                case GLFW_KEY_DELETE: {
                    delete_selection_forward(buffer, cursor_index, selection_distance, buffer_length);
                } break;
                case GLFW_KEY_BACKSPACE: {
                    delete_selection_backward(buffer, cursor_index, selection_distance, buffer_length);
                } break;
                case GLFW_KEY_ENTER: {
                    reset_active(ctx);
                } break;
                default: {
                    if (*selection_distance != 0) {
                        delete_selection_forward(buffer, cursor_index, selection_distance, buffer_length);
                    }
                    int length = ustring_unicode_to_utf8(key, utf8_key);
                    if(*buffer_length + length < buffer_max_length) {
                        memcpy(buffer + *cursor_index + length, buffer + *cursor_index, *buffer_length - *cursor_index);
                        memcpy(buffer + *cursor_index, utf8_key, length);
                        (*cursor_index) += length;
                        (*buffer_length) += length;
                    } else {
                        result = false;
                    }
                } break;
            }

            if (!(mods & GLFW_MOD_SHIFT)) {
                *selection_distance = 0;
            }
        }
    } else if(hot(ctx, id)) {
        if(input_mouse_button_went_down(MOUSE_LEFT_BUTTON, 0, 0)) {
            set_active(ctx,id);
        }
    }

    vec2 position = (vec2){200, 10};
    r32 width = 80.0f;
    r32 height = 30.0f;

    if(input_inside(input_mouse_position(), (vec4){position.x, position.y, width, height})) {
        set_hot(ctx, id);
    }

    // -------------------------------------------
    // ---------------- Draw ---------------------
    // -------------------------------------------
    vec4 color = (vec4){0.4f, 0.4f, 0.38f, 1.0f};
    if(active(ctx, id)) {
        color = (vec4){0.3f, 0.3f, 0.3f, 1.0f};
    } else if (hot(ctx, id)) {
        color = (vec4){0.4f, 0.4f, 0.3f, 1.0f};
    }
    Quad_2D q = quad_new(position, width, height, color);
    renderer_imm_quad(&q);

    extern Font_Info font_info;
    // Pre-render text
    Text_Render_Character_Position cursor_pos[2] = {0};
    int cindex = 0;
    int sindex = 0;
    if (*selection_distance < 0) {
        cindex++;
    } else {
        sindex++;
    }
    cursor_pos[cindex].index = *cursor_index;
    cursor_pos[sindex].index = *cursor_index + *selection_distance;

    Text_Render_Info info = text_prerender(&font_info, buffer, *buffer_length, cursor_pos, 2);
    vec2 text_position = (vec2){position.x + 2.0f, position.y + (height - font_info.max_height) / 2.0f };
    if(info.width - width > 0.0f) {
        text_position.x -= (info.width - width + 2.0f);
    }

    // Render text
    Clipping_Rect clipping = (Clipping_Rect) { position.x, position.y, width, height };
    text_render(&font_info, buffer, *buffer_length, text_position, clipping);

    if(active(ctx, id)) {
        // Render cursor
        renderer_imm_debug_box(
            cursor_pos[cindex].position.x + text_position.x + 1.0f,
            text_position.y - 3.0f, 
            1.0f, 
            font_info.max_height, 
            (vec4){1.0f, 1.0f, 1.0f, 1.0f});

        // Render Selection box if one exists
        if(*selection_distance != 0) {
            r32 selection_width = 0.0f;
            int min_index = 0;
            if(*selection_distance < 0) {
                selection_width = cursor_pos[cindex].position.x - cursor_pos[sindex].position.x;
                min_index = sindex;
            } else {
                selection_width = cursor_pos[sindex].position.x - cursor_pos[cindex].position.x;
                min_index = cindex;
            }
            renderer_imm_debug_box(
                cursor_pos[min_index].position.x + text_position.x, 
                text_position.y - 3.0f, 
                selection_width, 
                font_info.max_height, 
                (vec4){0.84f, 0.84f, 0.84f, 0.5f});
        }
    }

    return result;
}

void hg_window_begin(HG_Context* ctx, int id, vec2 position, r32 width, r32 height, const char* name) {
    Quad_2D q = quad_new(position, width, height, (vec4){0.5f, 0.5f, 0.5f, 1.0f});
    renderer_imm_quad(&q);
}