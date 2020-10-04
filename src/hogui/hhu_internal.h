#pragma once
#include "batcher.h"
#include "input.h"
#include "font_load.h"

#define HHU_USE_GLFW 1
#if defined(HHU_USE_GLFW)
#include <GLFW/glfw3.h>

typedef struct {
    GLFWwindow*     window;
    Hobatch_Context batch_ctx;
    Font_Info       font_info;
} HHU_Context;
#endif

extern HHU_Context* hhuctx;

void hhu_internal_init();
void hhu_internal_end();
bool hhu_is_hot(uint32_t id, int item, int index);
bool hhu_is_active(uint32_t id, int item, int index);
void hhu_set_active(uint32_t id, int item, int index);
void hhu_set_hot(uint32_t id, int item, int index);
void hhu_reset_hot_index();
void hhu_update_hot_conditionally(bool condition, uint32_t id, int item, int index);
void hhu_push_container_stack(float x, float y, float width, float height);

typedef enum {
    HGUI_WINDOW_FLAG_INITIALIZED = (1 << 0),
    HGUI_WINDOW_FLAG_LEFT_DRAG = (1 << 1),
    HGUI_WINDOW_FLAG_RIGHT_DRAG = (1 << 2),
    HGUI_WINDOW_FLAG_TOP_DRAG = (1 << 3),
    HGUI_WINDOW_FLAG_BOT_DRAG = (1 << 4),
    HGUI_WINDOW_FLAG_DRAGGING = (1 << 5),
} HGui_Window_Flags;