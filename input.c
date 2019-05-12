#include <GLFW/glfw3.h>
#include <stdio.h>
#include <locale.h>
#include "input.h"
#include "event.h"

typedef struct {
	r32 x;
	r32 y;
} Mouse_State;
Mouse_State input_mouse = { 0 };

static GLFWwindow* glfw_window;

void input_init(GLFWwindow* window) {
	glfw_window = window;
}

void window_get_size(s32* width, s32* height) {
	glfwGetFramebufferSize(glfw_window, width, height);
}

const char* input_get_clipboard() {
	return glfwGetClipboardString(glfw_window);
}

void
window_size_callback(GLFWwindow* window, s32 width, s32 height) {
	// use framebuffer size callback instead
}

static void
framebuffer_size_callback(GLFWwindow* window, s32 width, s32 height) {
	Event e = {0};
	e.type = EVENT_WINDOW;
	e.window.type = WINDOW_RESIZE;
	e.window.width = width;
	e.window.height = height;
	event_push(&e);
}

static void
window_focus_callback(GLFWwindow* window, s32 focused) {
	Event e = {0};
	e.type = EVENT_WINDOW;
	if (focused == GLFW_TRUE) {
		e.window.type = WINDOW_FOCUSED;
	} else {
		e.window.type = WINDOW_UNFOCUSED;
	}
	event_push(&e);
}

static void
key_callback(GLFWwindow* window, s32 key, s32 scancode, s32 action, s32 mods) {
	Event e = {0};
	e.type = EVENT_KEYBOARD_INPUT;
	e.keyboard.unicode = key;
	e.keyboard.mods = mods;
	e.keyboard.scancode = scancode;
	switch (action) {
		case GLFW_PRESS: {
			e.keyboard.type = KEYBOARD_KEY_PRESS;
		}break;
		case GLFW_RELEASE: {
			e.keyboard.type = KEYBOARD_KEY_RELEASE;
		}break;
		case GLFW_REPEAT:{
			e.keyboard.type = KEYBOARD_KEY_REPEAT;
		}break;
		default: return;
	}
	event_push(&e);
}


static void
cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	Event e = {0};
	e.type = EVENT_MOUSE_INPUT;
	e.mouse.type = MOUSE_POSITION;
	e.mouse.x = xpos;
	e.mouse.y = ypos;
	event_push(&e);

	input_mouse.x = (r32)xpos;
	input_mouse.y = (r32)ypos;

}

static void
mouse_button_callback(GLFWwindow* window, s32 button, s32 action, s32 mods) {
	Event e = {0};
	e.type = EVENT_MOUSE_INPUT;
	switch (action) {
		case GLFW_PRESS: {
			e.mouse.type = MOUSE_BUTTON_PRESS; 
			e.mouse.button = button;
			e.mouse.mods = mods;
			e.mouse.x = input_mouse.x;
			e.mouse.y = input_mouse.y;
		}break;
		case GLFW_RELEASE: {
			e.mouse.type = MOUSE_BUTTON_RELEASE;
			e.mouse.button = button;
			e.mouse.mods = mods;
			e.mouse.x = input_mouse.x;
			e.mouse.y = input_mouse.y;
		}break;
		default: return;
	}
	event_push(&e);
}

static void
scroll_callback(GLFWwindow* window, r64 xoffset, r64 yoffset) {
	Event e = {0};
	e.type = EVENT_MOUSE_INPUT;
	e.mouse.type = MOUSE_WHEEL;
	e.mouse.wheel_value = (r32)yoffset;
	event_push(&e);
}

static void
charmods_callback(GLFWwindow* window, u32 codepoint, s32 mods) {
	//printf("Character %lc %u\n", codepoint, codepoint);
	Event e = {0};
	e.type = EVENT_KEYBOARD_INPUT;
	e.keyboard.mods = mods;
	e.keyboard.type = KEYBOARD_CHAR;
	e.keyboard.unicode = codepoint;
	event_push(&e);
}

static void
cursor_enter_callback(GLFWwindow* window, s32 entered) {
	Event e = {0};
	e.type = EVENT_MOUSE_INPUT;
	if (entered) {
		e.mouse.type = MOUSE_ENTER_WINDOW;
	} else {
		e.mouse.type = MOUSE_LEFT_WINDOW;
	}
	event_push(&e);
}

// TODO(psv): deep copy of paths to generate event
// figure out how to free this memory later
static void
window_drop_callback(GLFWwindow* window, s32 count, const char** paths) {
	for (s32 i = 0; i < count; ++i) {
		// handle
	}
	r64 x, y;
	glfwGetCursorPos(window, &x, &y);
}

static GLFWcursor* glfw_cursor_arrow;
static GLFWcursor* glfw_cursor_hand;
static GLFWcursor* glfw_cursor_vresize;
static GLFWcursor* glfw_cursor_hresize;
static GLFWcursor* glfw_cursor_ibeam;
static GLFWcursor* glfw_cursor_crosshair;

void
input_set_callbacks(GLFWwindow* window) {
	glfw_window = window;
	setlocale(LC_ALL, "C.UTF-8");

	glfwSetKeyCallback(window, key_callback);
	glfwSetCharModsCallback(window, charmods_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetCursorEnterCallback(window, cursor_enter_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetWindowSizeCallback(window, window_size_callback);
	glfwSetWindowFocusCallback(window, window_focus_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetDropCallback(window, window_drop_callback);

	glfw_cursor_arrow = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	glfw_cursor_hand = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
	glfw_cursor_vresize = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
	glfw_cursor_hresize = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
	glfw_cursor_ibeam = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
	glfw_cursor_crosshair = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
}

Input_State input_state;

void 
input_set_state(Input_State state) {
	input_state = state;
}

void
input_cycle_state() {
	if(input_state == INPUT_STATE_GAME) {
		input_state = INPUT_STATE_OVERLAY;
		glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	} else if(input_state == INPUT_STATE_OVERLAY) {
		input_state = INPUT_STATE_GAME;
		//glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}

static bool
is_mod_key(u32 key) {
	return (key == GLFW_MOD_CONTROL || 
		key == GLFW_MOD_ALT || 
		key == GLFW_MOD_SHIFT || 
		key == GLFW_MOD_SUPER||
		key == GLFW_MOD_CAPS_LOCK);
}

static char* key_names[1024];

static void
input_print_keys(u32 keys[3]) {
	for (s32 i = 0; i < 3; ++i) {
		if (keys[i]) {
			if (i > 0) printf(" + ");
			if (key_names[keys[i]]) printf("%s", key_names[keys[i]]);
			else printf("%c", keys[i]);
		} else {
			break;
		}
	}
}