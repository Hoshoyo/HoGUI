#pragma once
#include <GLFW/glfw3.h>
#include "common.h"

typedef enum {
	ARROW_NONE = 0,
	ARROW_LEFT,
	ARROW_RIGHT,
	ARROW_UP,
	ARROW_DOWN,
} Input_ArrowKeys;

typedef enum {
	INPUT_STATE_OVERLAY,
	INPUT_STATE_GAME,
} Input_State;

extern Input_State input_state;

void        input_set_callbacks(GLFWwindow* window);
const char* input_get_clipboard();
void        input_set_clipboard(const char* text, int length);
void        input_set_state(Input_State state);
void        input_cycle_state();
void        input_get_mouse_pos(r32* x, r32* y);

// Window functions
void      window_get_size(s32* width, s32* height);

typedef enum {
	KEY_MOD_NONE = 0,
	KEY_MOD_SHIFT,
	KEY_MOD_CTRL,
	KEY_MOD_LALT,
	KEY_MOD_RALT,
} Key_Modifiers;

void input_init(GLFWwindow*);