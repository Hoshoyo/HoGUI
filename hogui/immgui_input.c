#include "immgui_input.h"
#include "../event.h"
#include <gm.h>
#include <string.h>

#define MOUSE_BUTTON_COUNT 8
#define KEYBOARD_KEY_COUNT 1024

typedef struct {
    int width, height;
} Input_Window;

typedef struct {
    int x, y;
    bool button_state[MOUSE_BUTTON_COUNT];
    int button_went_down[MOUSE_BUTTON_COUNT];
    int button_went_up[MOUSE_BUTTON_COUNT];
} Input_Mouse;

typedef struct {
    int key_state[KEYBOARD_KEY_COUNT];
    int key_went_up[KEYBOARD_KEY_COUNT];
    int key_went_down[KEYBOARD_KEY_COUNT];
} Input_Keyboard;

typedef struct {
    Input_Mouse mouse;
    Input_Keyboard keyboard;

    Input_Window window;
} Input_State;

static Input_State input_state;

void input_immgui() {
    // Reset input events
    memset(input_state.keyboard.key_went_down, 0, sizeof(input_state.keyboard.key_went_down));
    memset(input_state.keyboard.key_went_up, 0, sizeof(input_state.keyboard.key_went_up));

    memset(input_state.mouse.button_went_down, 0, sizeof(input_state.mouse.button_went_down));
    memset(input_state.mouse.button_went_up, 0, sizeof(input_state.mouse.button_went_up));

    Event e;
    while (event_pop(&e)) {
        if(e.type == EVENT_KEYBOARD_INPUT) {
            switch(e.keyboard.type) {
                case KEYBOARD_KEY_PRESS: {
                    input_state.keyboard.key_state[e.keyboard.unicode] = true;
                    input_state.keyboard.key_went_down[e.keyboard.unicode] += 1;
                } break;
                case KEYBOARD_KEY_RELEASE: {
                    input_state.keyboard.key_state[e.keyboard.unicode] = false;
                    input_state.keyboard.key_went_up[e.keyboard.unicode] += 1;
                } break;
                default: break;
            }
        } else if(e.type == EVENT_MOUSE_INPUT) {
            switch(e.mouse.type) {
                case MOUSE_BUTTON_PRESS: {
                    input_state.mouse.button_state[e.mouse.button] = true;
                    input_state.mouse.button_went_down[e.mouse.button] += 1;
                }break;
                case MOUSE_BUTTON_RELEASE: {
                    input_state.mouse.button_state[e.mouse.button] = false;
                    input_state.mouse.button_went_up[e.mouse.button] += 1;
                }break;
                case MOUSE_POSITION: {
                    input_state.mouse.x = (int)e.mouse.x;
                    input_state.mouse.y = input_state.window.height - (int)e.mouse.y;
                }break;
                default:break;
            }
        } else if(e.type == EVENT_WINDOW) {
            switch(e.window.type) {
                case WINDOW_RESIZE: {
                    input_state.window.width = e.window.width;
                    input_state.window.height = e.window.height;
                } break;
                default: break;
            }
        }
    }
}

bool input_is_key_down(u32 key) {
    if(key >= 0 && key < KEYBOARD_KEY_COUNT)
        return input_state.keyboard.key_state[key];
    return false;
}

bool input_is_mouse_button_down(int button) {
    if(button >= 0 && button < MOUSE_BUTTON_COUNT)
        return input_state.mouse.button_state[button];
    return false;
}

int input_key_went_down(u32 key) {
    if(key >= 0 && key < KEYBOARD_KEY_COUNT)
        return input_state.keyboard.key_went_down[key];
}
int input_key_went_up(u32 key) {
    if(key >= 0 && key < KEYBOARD_KEY_COUNT)
        return input_state.keyboard.key_went_up[key];
}
int input_mouse_button_went_down(int button, int* x, int* y) {
    if(button >= 0 && button < MOUSE_BUTTON_COUNT) {
        if(x) *x = input_state.mouse.x;
        if(y) *y = input_state.mouse.y;
        return input_state.mouse.button_went_down[button];
    }
}
int input_mouse_button_went_up(int button, int* x, int* y) {
    if(button >= 0 && button < MOUSE_BUTTON_COUNT) {
        if(x) *x = input_state.mouse.x;
        if(y) *y = input_state.mouse.y;
        return input_state.mouse.button_went_up[button];
    }
}
vec2 input_mouse_position() {
    return (vec2){(r32)input_state.mouse.x, (r32)input_state.mouse.y};
}

bool input_inside(vec2 p, vec4 clipping) {
    return (
        (p.x >= clipping.x && p.x <= clipping.x + clipping.z) && 
        (p.y >= clipping.y && p.y <= clipping.y + clipping.w)
    );
}

// clipping (vec4) {x, y, width, height}
bool input_inside_border(vec2 p, vec4 clipping, r32 slack, u32* out_flags) {
	r32 left = clipping.x;
	r32 right = clipping.x + clipping.z;
	r32 bot = clipping.y;
	r32 top = clipping.y + clipping.w;

	bool touching_left = (p.x >= left - slack) && (p.x <= left + slack) && (p.y >= bot - slack) && (p.y <= top + slack);
	bool touching_right = (p.x >= right - slack) && (p.x <= right + slack) && (p.y >= bot - slack) && (p.y <= top + slack);
	bool touching_top = (p.y >= top - slack) && (p.y <= top + slack) && (p.x >= left - slack) && (p.x <= right + slack);
	bool touching_bot = (p.y >= bot - slack) && (p.y <= bot + slack) && (p.x >= left - slack) && (p.x <= right + slack);

	if (touching_left) *out_flags  |= INPUT_SELECTING_BORDER_LEFT;
	if (touching_right) *out_flags |= INPUT_SELECTING_BORDER_RIGHT;
	if (touching_bot) *out_flags   |= INPUT_SELECTING_BORDER_BOTTOM;
	if (touching_top) *out_flags   |= INPUT_SELECTING_BORDER_TOP;

	return touching_left || touching_right || touching_bot || touching_top;
}