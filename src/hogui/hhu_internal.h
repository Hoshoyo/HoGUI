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