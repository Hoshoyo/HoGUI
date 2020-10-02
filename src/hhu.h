#pragma once

typedef void HHU_Context;

HHU_Context* hhu_init();
int          hhu_destroy(HHU_Context* ctx);
void         hhu_render(HHU_Context* ctx);

#if defined(HHU_USE_GLFW)
#include <GLFW/glfw3.h>
void hhu_init_glfw(HHU_Context* ctx, GLFWwindow* window);
#endif

// Input functions
int hhu_input_init(HHU_Context* ctx);
int hhu_input_clear(HHU_Context* ctx);