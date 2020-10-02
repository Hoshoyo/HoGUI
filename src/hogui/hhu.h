#pragma once

typedef void HHU_Context;

HHU_Context* hhu_init();
int          hhu_destroy(HHU_Context* ctx);
void         hhu_render(HHU_Context* ctx);

//#if defined(HHU_USE_GLFW)
#include <GLFW/glfw3.h>
void hhu_glfw_init(HHU_Context* ctx, GLFWwindow* window);
void hhu_glfw_set_callback_cursor_pos(GLFWcursorposfun cb);
void hhu_glfw_set_callback_cursor_enter(GLFWcursorenterfun cb);
void hhu_glfw_set_callback_key(GLFWkeyfun cb);
void hhu_glfw_set_callback_char(GLFWcharfun cb);
void hhu_glfw_set_callback_mouse_button(GLFWmousebuttonfun cb);
void hhu_glfw_set_callback_scroll(GLFWscrollfun cb);
void hhu_glfw_set_callback_drop(GLFWdropfun cb);
void hhu_glfw_set_callback_frame_buffer_size(GLFWframebuffersizefun cb);
void hhu_glfw_set_callback_window_pos(GLFWwindowposfun cb);
//#endif

// Input functions
int hhu_input_init(HHU_Context* ctx);
int hhu_input_clear(HHU_Context* ctx);