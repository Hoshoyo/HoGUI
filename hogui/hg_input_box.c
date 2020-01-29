#include "hg_ui.h"
#include "../renderer/renderer_imm.h"
#include "hg_input.h"
#include "hg_internal.h"
#include "../input.h"

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

static int insert_text(const char* clip, int clipboard_length, char* buffer, int buffer_max_length, int* cursor_index, int* selection_distance, int* buffer_length) {
    int insert_max = buffer_max_length - *buffer_length + (*selection_distance < 0 ? -(*selection_distance) : *selection_distance);
    if(*selection_distance == 0) {
        
        int inserted_length = MIN(insert_max, clipboard_length);
        memcpy(buffer + *cursor_index + inserted_length, buffer + *cursor_index, inserted_length);
        memcpy(buffer + *cursor_index, clip, inserted_length);
        *cursor_index += inserted_length;
        *buffer_length += inserted_length;
        return inserted_length;
    }
    return 0;
}

static HG_Input_Box_Exit
handle_input(char* buffer, int* buffer_length, int buffer_max_length, int* cursor_index, int* selection_distance) {
    HG_Input_Box_Exit exit_mode = HG_INPUT_BOX_EXIT_NONE;
    u32 key = 0;
    s32 mods = 0;
    char utf8_key[4] = {0};
    bool reset_selection = true;

    while(input_next_key_pressed(&key, &mods)) {
        switch(key) {
            case GLFW_KEY_ESCAPE: {
                exit_mode = HG_INPUT_BOX_EXIT_ESCAPE;
            } break;
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
            case GLFW_KEY_DOWN:
            case GLFW_KEY_UP:
                // ignore these
                break;
            case GLFW_KEY_LEFT: {
                if(!(mods & GLFW_MOD_SHIFT)) {
                    if(*selection_distance > 0) {
                        *selection_distance = 0;
                        break;
                    } else if(*selection_distance < 0) {
                        *cursor_index += *selection_distance;
                        *selection_distance = 0;
                        break;
                    }
                }
                if(*cursor_index > 0) {
                    int l = length_current_utf8(buffer + *cursor_index - 1);
                    (*cursor_index) -= l;
                    if(mods & GLFW_MOD_SHIFT) {
                        *selection_distance += l;
                    }
                }
            } break;
            case GLFW_KEY_RIGHT: {
                if(!(mods & GLFW_MOD_SHIFT)) {
                    if(*selection_distance > 0) {
                        *cursor_index += *selection_distance;
                        *selection_distance = 0;
                        break;
                    } else if(*selection_distance < 0) {
                        *selection_distance = 0;
                        break;
                    }
                }
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
                exit_mode = HG_INPUT_BOX_EXIT_ENTER;
            } break;
            default: {
                
                if(mods & GLFW_MOD_CONTROL) {
                    if(key == 'V') {
                        if (*selection_distance != 0) {
                            delete_selection_forward(buffer, cursor_index, selection_distance, buffer_length);
                        }
                        const char* clip = input_get_clipboard();
                        int clipboard_length = strlen(clip);
                        insert_text(clip, clipboard_length, buffer, buffer_max_length, cursor_index, selection_distance, buffer_length);
                    }
                    if(key == 'C') {
                        if(*selection_distance > 0) {
                            input_set_clipboard(buffer + *cursor_index, *selection_distance);
                        } else if(*selection_distance < 0) {
                            input_set_clipboard(buffer + *cursor_index + *selection_distance, -(*selection_distance));
                        }
                        reset_selection = false;
                    }
                    break;
                }
                
                if (*selection_distance != 0) {
                    delete_selection_forward(buffer, cursor_index, selection_distance, buffer_length);
                }

                int length = ustring_unicode_to_utf8(key, utf8_key);
                if(*buffer_length + length < buffer_max_length) {
                    memcpy(buffer + *cursor_index + length, buffer + *cursor_index, *buffer_length - *cursor_index);
                    memcpy(buffer + *cursor_index, utf8_key, length);
                    (*cursor_index) += length;
                    (*buffer_length) += length;
                }
            } break;
        }

        if (!(mods & GLFW_MOD_SHIFT) && reset_selection) {
            *selection_distance = 0;
        }
    }
    return exit_mode;
}

static Clipping_Rect 
input_box_render_auto_layout(HG_Context* ctx, s64 id, int item, char* buffer, int buffer_length, int selection_distance, int cursor_index) {
    // Design parameters
    vec4 color = (vec4){0.4f, 0.4f, 0.38f, 1.0f};
    vec4 color_active = (vec4){0.3f, 0.3f, 0.3f, 1.0f};
    vec4 color_hot = (vec4){0.4f, 0.4f, 0.3f, 1.0f};
    vec4 color_cursor = (vec4){1.0f, 1.0f, 1.0f, 1.0f};
    vec4 color_sel_box = (vec4){0.84f, 0.84f, 0.84f, 0.5f};
    vec4 color_text = (vec4){1.0f, 1.0f, 1.0f, 1.0f};
    color.a = 0.5f;
    color_active.a = 0.5f;
    color_hot.a = 0.5f;

    // Positioning metrics
    vec2          position = (vec2){0, 0};
    Clipping_Rect input_clipping = {0};
    r32           width = 0.0f;
    r32           height = 25.0f;
    r32           border_width = 1.0f;

    // Calculate the layout in the current context
    hg_layout_rectangle_top_down(ctx, &position, &width, &height, &input_clipping);

    // Merge the clipping box of the input box with the context clipping
    input_clipping = clipping_rect_merge(input_clipping, (vec4){position.x, position.y, width, height});

    // Compensate for borders
    width -= (2.0f * border_width);
    position.x += border_width;
    position.y += border_width;
    height -= (2.0f * border_width);

    // Draw
    
    if(active_item(ctx, id, item)) {
        color = color_active;
    } else if (hot_item(ctx, id, item)) {
        color = color_hot;
    }

    Quad_2D q = quad_new_clipped(position, width, height, color, input_clipping);
    renderer_imm_quad(&q);

    vec4 input_border_color[] = {(vec4){0.2f, 0.2f, 0.2f, 1.0f}, (vec4){0.6f, 0.6f, 0.6f, 1.0f}, (vec4){0.5f, 0.5f, 0.5f, 1.0f}, (vec4){0.2f, 0.2f, 0.2f, 1.0f}};
    renderer_imm_outside_border_v1(q, border_width, input_border_color);

    // Pre-render text and get position information for both
    // cursor index and selection index rendering positions.
    Text_Render_Character_Position cursor_pos[2] = {0};
    int cindex = (selection_distance < 0) ? 1 : 0;  // "cursor_index" information index
    int sindex = (selection_distance < 0) ? 0 : 1;  // "selection_distance" information index
    cursor_pos[cindex].index = cursor_index;
    cursor_pos[sindex].index = cursor_index + selection_distance;

    // Figure out text layout from the current position
    Text_Render_Info info = text_prerender(&ctx->font_info, buffer, buffer_length, 0, cursor_pos, 2);
    vec2 text_position = (vec2){position.x + 2.0f, position.y + (height - ctx->font_info.max_height) / 2.0f };

    // If the cursor is outside the view, bring it into view
    if(cursor_pos[cindex].position.x > width) {
        text_position.x -= (cursor_pos[cindex].position.x - width + ctx->font_info.max_width + info.width - cursor_pos[cindex].position.x);
    }

    // Render text in the calculated position
    text_render(&ctx->font_info, buffer, buffer_length, 0, text_position, input_clipping, color_text);

    if(active_item(ctx, id, item)) {
        // Render cursor
        Quad_2D cursor_quad = quad_new_clipped(
            (vec2){cursor_pos[cindex].position.x + text_position.x + 1.0f, text_position.y - 3.0f}, // position
            1.0f, // width
            ctx->font_info.max_height, // height
            color_cursor, input_clipping);
        renderer_imm_quad(&cursor_quad);

        // Render Selection box if one exists
        if(selection_distance != 0) {
            int min_index = (selection_distance < 0) ? sindex : cindex;
            int max_index = (selection_distance < 0) ? cindex : sindex;
            r32 selection_width = cursor_pos[max_index].position.x - cursor_pos[min_index].position.x;

            Quad_2D select_box_quad = quad_new((vec2){cursor_pos[min_index].position.x + text_position.x, text_position.y - 3.0f}, selection_width, ctx->font_info.max_height, color_sel_box);
            Quad_2D select_box_quad_clipped = quad_new_clipped((vec2){cursor_pos[min_index].position.x + text_position.x, text_position.y - 3.0f}, selection_width, ctx->font_info.max_height, color_sel_box, input_clipping);
            renderer_imm_quad(&select_box_quad_clipped);
            renderer_imm_border_clipped_simple(select_box_quad, 1.0f, color_sel_box, input_clipping);
        }
    }

    return input_clipping;
}

// selection_distance == 0 means no selection is active
HG_Input_Box_Exit hg_do_input(HG_Context* ctx, s64 id, int item, char* buffer, int buffer_max_length, int* buffer_length, int* cursor_index, int* selection_distance) {
    hg_update(ctx);
    HG_Input_Box_Exit result = false;
    
    if(active_item(ctx, id, item)) {
        if((result = handle_input(buffer, buffer_length, buffer_max_length, cursor_index, selection_distance)) != 0) {
            hg_reset_active(ctx);
        }
    } else if(hot_item(ctx, id, item)) {
        if(input_mouse_button_went_down(MOUSE_LEFT_BUTTON, 0, 0)) {
            hg_set_active(ctx, id, item);
        }
    }

    Clipping_Rect clipping = input_box_render_auto_layout(ctx, id, item, buffer, *buffer_length, *selection_distance, *cursor_index);

    if(input_inside(input_mouse_position(), clipping)) {
        set_hot(ctx, id, item);
    } else if(hot_item(ctx, id, item)) {
        reset_hot(ctx);
    }

    return result;
}