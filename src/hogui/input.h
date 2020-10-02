#pragma once
#include <stdint.h>
#include <GLFW/glfw3.h>

typedef enum {
    HINP_EVENT_NONE = 0,

    // mouse events
    HINP_EVENT_MOUSE_MOVE,
    HINP_EVENT_MOUSE_CLICK,
    HINP_EVENT_MOUSE_SCROLL,
    HINP_EVENT_MOUSE_LEAVE,
    HINP_EVENT_MOUSE_ENTER,

    // key events
    HINP_EVENT_KEYBOARD,

    // char events
    HINP_EVENT_CHARACTER,

    // window events
    HINP_EVENT_WINDOW_RESIZE,
    HINP_EVENT_WINDOW_MOVE,
} Hinp_Event_Type;

typedef enum {
    HINP_MOD_SHIFT = (1 << 0),
    HINP_MOD_CTRL  = (1 << 1),
    HINP_MOD_ALT   = (1 << 2),
    HINP_MOD_SUPER = (1 << 3),
} Hinp_Mods;

/* Window */
typedef struct {
    float x, y;
    float width, height;
} Hinp_Window;

/* Mouse */
typedef struct {
    float x, y;             // current x and y coords
    float px, py;           // previous x and y
    float deltax, deltay;   // difference from last frame position

    float scroll_delta_x;   // < 0 scroll down, > 0 scroll up, 0 = no scroll
    float scroll_delta_y;   // < 0 scroll down, > 0 scroll up, 0 = no scroll

    int       button;       // 0 Left, 1 Right, 2 Middle
    int       action;       // 1 down, 0 up
    Hinp_Mods mods;
} Hinp_Mouse;

/* Keyboard */
typedef struct {
    uint8_t  utf8[4];
    uint8_t  utf8_num_bytes;
    uint32_t unicode;
} Hinp_Character;

typedef struct {
    int       key;
    int       scancode;
    int       action;   /* 1 press, 0 release, 2 repeat */
    Hinp_Mods mods;
} Hinp_Event_Keyboard;

typedef struct {
    Hinp_Event_Type     type;
    Hinp_Event_Keyboard keyboard;
    Hinp_Mouse          mouse;
    Hinp_Window         window;
    Hinp_Character      character;
} Hinp_Event;

int  hinp_init(GLFWwindow* window);
void hinp_destroy();
void hinp_window_size(float* width, float* height);

int hinp_event_next(Hinp_Event* ev);

int hinp_query_mouse_inside();