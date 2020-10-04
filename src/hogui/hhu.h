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

// -----------------------------------------
// Stateful Widgets ------------------------

typedef enum {
    HGUI_WINDOW_STYLE_DARK,
    HGUI_WINDOW_STYLE_LIGHT,
} HGui_Window_Style;

typedef struct {
    int               id;
    uint32_t          flags;
    HGui_Window_Style style;
    float             x, y;
    float             width, height;
    float             header_size_px;
    float             border_size_px;
} HGui_Window;

// Core
void hhu_begin();
void hhu_end();
void hhu_window(HGui_Window* window, char* name);

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