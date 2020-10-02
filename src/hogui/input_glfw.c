#include <stdio.h>
#include "input.h"
#include <GLFW/glfw3.h>
#define MAX_EVENTS 32
typedef struct {
    GLFWcursorposfun       callback_cursor_pos;
    GLFWcursorenterfun     callback_cursor_enter;
    GLFWkeyfun             callback_key;
    GLFWcharfun            callback_char;
    GLFWmousebuttonfun     callback_mouse_button;
    GLFWscrollfun          callback_mouse_scroll;
    GLFWdropfun            callback_drop;
    GLFWframebuffersizefun callback_framebuffer_size;
    GLFWwindowposfun       callback_window_pos;
} Hinp_GLFW_Callbacks;

typedef struct {
    // event queue is a circular array
    Hinp_Event events[MAX_EVENTS];
    int        event_index;
    int        event_count;

    // Mouse info
    float mouse_x, mouse_y;
    int   mouse_inside;

    // Window info
    float window_width, window_height;
    float window_x, window_y;

    GLFWwindow* glfw_window;
    Hinp_GLFW_Callbacks callbacks;
} Hinp_Ctx;
static Hinp_Ctx ctx;

static void
event_push(Hinp_Event* ev)
{
    int index = (ctx.event_index + ctx.event_count) % MAX_EVENTS;
    ctx.events[index] = *ev;
    if(ctx.event_count == MAX_EVENTS)
        ctx.event_index = (ctx.event_index + 1) % MAX_EVENTS;
    else
        ctx.event_count++;
}

int
hinp_event_next(Hinp_Event* ev)
{
    if(ctx.event_count == 0) return 0;

    *ev = ctx.events[ctx.event_index];
    ctx.event_index = (ctx.event_index + 1) % MAX_EVENTS;
    ctx.event_count--;
    return ctx.event_count + 1;
}

/* UTF8 */

static int
unicode_to_utf8(uint32_t unicode, uint8_t* buffer)
{
  char* start = buffer;
  char* result = buffer;
  {
    if (unicode <= 0x7f)
    {
      *result++ = (uint8_t) unicode;
    }
    else if (unicode >= 0x80 && unicode <= 0x7ff)
    {
      uint8_t b1 = 0xc0 | (unicode >> 6);
      uint8_t b2 = 0x80 | ((unicode & 0x3f) | 0x30000000);
      *result++ = b1;
      *result++ = b2;
    }
    else if (unicode >= 0x800 && unicode <= 0xffff)
    {
      uint8_t b1 = 0xe0 | (unicode >> 12);
      uint8_t b2 = 0x80 | (((unicode >> 6) & 0x3f) | 0x30000000);
      uint8_t b3 = 0x80 | ((unicode & 0x3f) | 0x30000000);
      *result++ = b1;
      *result++ = b2;
      *result++ = b3;
    }
    else if (unicode >= 0x00010000 && unicode <= 0x001fffff)
    {
      uint8_t b1 = 0xf0 | (unicode >> 18);
      uint8_t b2 = 0x80 | (((unicode >> 12) & 0x3f) | 0x30000000);
      uint8_t b3 = 0x80 | (((unicode >> 6) & 0x3f) | 0x30000000);
      uint8_t b4 = 0x80 | ((unicode & 0x3f) | 0x30000000);
      *result++ = b1;
      *result++ = b2;
      *result++ = b3;
      *result++ = b4;
    }
  }
  return (int) (result - start);
}


/* -------------------------------------------- */

static void
cursor_enter_callback(GLFWwindow* window, int entered)
{
    Hinp_Event ev = {
        .type = (entered) ? HINP_EVENT_MOUSE_ENTER : HINP_EVENT_MOUSE_LEAVE,
        .mouse.x  = ctx.mouse_x,
        .mouse.y  = ctx.mouse_y,
    };
    ctx.mouse_inside = entered;
    event_push(&ev);

    if(ctx.callbacks.callback_cursor_enter)
        ctx.callbacks.callback_cursor_enter(window, entered);
}

static void
mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    Hinp_Event ev = {
        .type = HINP_EVENT_MOUSE_SCROLL,
        .mouse.x  = ctx.mouse_x,
        .mouse.y  = ctx.mouse_y,
        .mouse.action = action,
        .mouse.mods = mods,
        .mouse.button = button,
    };
    event_push(&ev);

    if(ctx.callbacks.callback_mouse_button)
        ctx.callbacks.callback_mouse_button(window, button, action, mods);
}

static void
scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    Hinp_Event ev = {
        .type = HINP_EVENT_MOUSE_SCROLL,
        .mouse.x  = ctx.mouse_x,
        .mouse.y  = ctx.mouse_y,
        .mouse.scroll_delta_x = (float)xoffset,
        .mouse.scroll_delta_y = (float)yoffset,
    };

    event_push(&ev);

    if(ctx.callbacks.callback_mouse_scroll)
        ctx.callbacks.callback_mouse_scroll(window, xoffset, yoffset);
}

static void
cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    Hinp_Event ev = {
        .type = HINP_EVENT_MOUSE_MOVE,
        .mouse.x  = (float)xpos,
        .mouse.y  = (float)ypos,
        .mouse.px = ctx.mouse_x,
        .mouse.py = ctx.mouse_y,
        .mouse.deltax = (float)xpos - ctx.mouse_x,
        .mouse.deltay = (float)ypos - ctx.mouse_y,
    };
    ctx.mouse_x = (float)xpos;
    ctx.mouse_y = (float)ypos;

    event_push(&ev);

    if(ctx.callbacks.callback_cursor_pos)
        ctx.callbacks.callback_cursor_pos(window, xpos, ypos);
}

static void
key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Hinp_Event ev = {
        .type = HINP_EVENT_KEYBOARD,
        .keyboard.key = key,
        .keyboard.action = action,
        .keyboard.mods = mods,
        .keyboard.scancode = scancode
    };
    event_push(&ev);

    if(ctx.callbacks.callback_key)
        ctx.callbacks.callback_key(window, key, scancode, action, mods);
}

static void
character_callback(GLFWwindow* window, unsigned int codepoint)
{
    Hinp_Event ev = {
        .type = HINP_EVENT_CHARACTER,
        .character.unicode = codepoint,
    };
    ev.character.utf8_num_bytes = (uint8_t)unicode_to_utf8(codepoint, ev.character.utf8);
    event_push(&ev);

    if(ctx.callbacks.callback_char)
        ctx.callbacks.callback_char(window, codepoint);
}

static void
drop_callback(GLFWwindow* window, int count, const char** paths)
{
    if(ctx.callbacks.callback_char)
        ctx.callbacks.callback_drop(window, count, paths);
}

static void
framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    Hinp_Event ev = {
        .type = HINP_EVENT_WINDOW_RESIZE,
        .window.x = ctx.window_x,
        .window.y = ctx.window_y,
        .window.width = (float)width,
        .window.height = (float)height,
    };
    ctx.window_width = (float)width;
    ctx.window_height = (float)height;
    event_push(&ev);

    if(ctx.callbacks.callback_framebuffer_size)
        ctx.callbacks.callback_framebuffer_size(window, width, height);
}

static void
window_pos_callback(GLFWwindow* window, int xpos, int ypos)
{
    Hinp_Event ev = {
        .type = HINP_EVENT_WINDOW_MOVE,
        .window.x = (float)xpos,
        .window.y = (float)ypos,
        .window.width = ctx.window_width,
        .window.height = ctx.window_height,
    };
    ctx.window_x = (float)xpos;
    ctx.window_y = (float)ypos;
    event_push(&ev);

    if(ctx.callbacks.callback_window_pos)
        ctx.callbacks.callback_window_pos(window, xpos, ypos);
}

int
hinp_init(GLFWwindow* window)
{
    ctx.glfw_window = window;

    // initialize mouse
    double mx = 0.0, my = 0.0;
    glfwGetCursorPos(window, &mx, &my);
    ctx.mouse_x = (float)mx;
    ctx.mouse_y = (float)my;

    // initialize window
    int ww = 0, wh = 0;
    glfwGetFramebufferSize(window, &ww, &wh);
    ctx.window_width = (float)ww;
    ctx.window_height = (float)wh;

    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, character_callback);
    glfwSetCursorEnterCallback(window, cursor_enter_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetDropCallback(window, drop_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetWindowPosCallback(window, window_pos_callback);
    return 0;
}

void
hinp_window_size(float* width, float* height)
{
    int w = 0, h = 0;
    glfwGetFramebufferSize(ctx.glfw_window, &w, &h);
    *width = w;
    *height = h;
}

void
hhu_glfw_set_callback_cursor_pos(GLFWcursorposfun cb)
{
    ctx.callbacks.callback_cursor_pos = cb;
}

void
hhu_glfw_set_callback_cursor_enter(GLFWcursorenterfun cb)
{
    ctx.callbacks.callback_cursor_enter = cb;
}

void
hhu_glfw_set_callback_key(GLFWkeyfun cb)
{
    ctx.callbacks.callback_key = cb;
}

void
hhu_glfw_set_callback_char(GLFWcharfun cb)
{
    ctx.callbacks.callback_char = cb;
}

void
hhu_glfw_set_callback_mouse_button(GLFWmousebuttonfun cb)
{
    ctx.callbacks.callback_mouse_button = cb;
}

void
hhu_glfw_set_callback_scroll(GLFWscrollfun cb)
{
    ctx.callbacks.callback_mouse_scroll = cb;
}

void
hhu_glfw_set_callback_drop(GLFWdropfun cb)
{
    ctx.callbacks.callback_drop = cb;
}

void
hhu_glfw_set_callback_frame_buffer_size(GLFWframebuffersizefun cb)
{
    ctx.callbacks.callback_framebuffer_size = cb;
}

void
hhu_glfw_set_callback_window_pos(GLFWwindowposfun cb)
{
    ctx.callbacks.callback_window_pos = cb;
}

// ----

int
hinp_query_mouse_inside()
{
    return ctx.mouse_inside;
}