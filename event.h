#pragma once
#include "common.h"

typedef enum {
	EVENT_STATUS_HANDLED = 0,
	EVENT_STATUS_UNHANDLED = 1,
} Event_Handle_Status;

typedef enum {
	EVENT_NULL = 0,
	EVENT_KEYBOARD_INPUT,
	EVENT_MOUSE_INPUT,
	EVENT_WINDOW,
} Event_Type;

typedef enum {
	KEYBOARD_KEY_PRESS,
	KEYBOARD_KEY_RELEASE,
	KEYBOARD_KEY_REPEAT,
	KEYBOARD_CHAR,
} Keyboard_Event_Type;

typedef struct {
	Keyboard_Event_Type type;
	u32 unicode;
	s32 scancode;
	s32 mods;
} Event_Keyboard_Input;

typedef enum {
	MOUSE_BUTTON_PRESS,
	MOUSE_BUTTON_RELEASE,
	MOUSE_POSITION,
	MOUSE_WHEEL,
	MOUSE_LEFT_WINDOW,
	MOUSE_ENTER_WINDOW,
} Mouse_Event_Type;

typedef struct {
	Mouse_Event_Type type;
	r64 x;
	r64 y;
	r32 wheel_value;
	s32 button;
	s32 mods;
} Event_Mouse_Input;

typedef enum {
	WINDOW_RESIZE,
	WINDOW_FOCUSED,
	WINDOW_UNFOCUSED,
} Window_Event_Type;

typedef struct {
	Window_Event_Type type;
	s32 width;
	s32 height;
} Event_Window;

typedef struct {
	Event_Type type;
	union {
		Event_Keyboard_Input keyboard;
		Event_Mouse_Input mouse;
		Event_Window window;
	};
} Event;

bool event_pop(Event* out_event);
void event_push(Event* event);
void event_queue_clear();
bool event_peek(Event* e, int index);