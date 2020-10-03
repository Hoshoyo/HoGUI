#pragma once
#include <stdint.h>
#include "common.h"

// Type definitions
typedef struct {
    float x, y;
} hhu_v2;

typedef struct {
    float x, y, z;
} hhu_v3;

typedef struct {
    float x, y, z, w;
} hhu_v4;

typedef struct {
    float r, g, b, a;
} hhu_color;

int hhu_init();
int hhu_destroy();
int hhu_render();

// Colors
extern hhu_color hhu_color_red;
extern hhu_color hhu_color_green;
extern hhu_color hhu_color_blue;

// Core
void hhu_window(float x, float y, float width, float height, hhu_color color);

bool hhu_is_hot(uint32_t id, int item, int index);
bool hhu_is_active(uint32_t id, int item, int index);

// -----------------------------------------
// GLFW ------------------------------------
#if defined(HHU_USE_GLFW)
#include <GLFW/glfw3.h>
void hhu_glfw_init(GLFWwindow* window);
void hhu_glfw_set_callback_cursor_pos(GLFWcursorposfun cb);
void hhu_glfw_set_callback_cursor_enter(GLFWcursorenterfun cb);
void hhu_glfw_set_callback_key(GLFWkeyfun cb);
void hhu_glfw_set_callback_char(GLFWcharfun cb);
void hhu_glfw_set_callback_mouse_button(GLFWmousebuttonfun cb);
void hhu_glfw_set_callback_scroll(GLFWscrollfun cb);
void hhu_glfw_set_callback_drop(GLFWdropfun cb);
void hhu_glfw_set_callback_frame_buffer_size(GLFWframebuffersizefun cb);
void hhu_glfw_set_callback_window_pos(GLFWwindowposfun cb);
#endif
