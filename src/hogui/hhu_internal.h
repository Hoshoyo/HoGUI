#pragma once
#include "batcher.h"
#include "input.h"

#define HHU_USE_GLFW 1
#if defined(HHU_USE_GLFW)
#include <GLFW/glfw3.h>

typedef struct {
    GLFWwindow* window;
    Hobatch_Context batch_ctx;
} HHU_Context;
#endif

extern HHU_Context* hhuctx;

bool hhu_is_hot(uint32_t id, int item, int index);
bool hhu_is_active(uint32_t id, int item, int index);
void hhu_set_active(uint32_t id, int item, int index);
void hhu_set_hot(uint32_t id, int item, int index);
void hhu_update_hot_conditionally(bool condition, uint32_t id, int item, int index);

typedef enum {
    HGUI_WINDOW_FLAG_INITIALIZED = (1 << 0),
} HGui_Window_Flags;