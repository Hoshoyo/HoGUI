#define HOGL_IMPLEMENT
#define GRAPHICS_MATH_IMPLEMENT
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <ho_gl.h>
#include <gm.h>
#include "input.h"
#include "event.h"
#include "renderer/renderer_imm.h"
#include "font/font_load.h"
#include "hogui/hogui.h"

static Font_Info font_info;

int main() {
    if (!glfwInit()) {
		printf("Could not initialize GLFW\n");
		return EXIT_FAILURE;
	}

	// Create a windowed mode window and its OpenGL context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWwindow* window = glfwCreateWindow(1024, 768, "Squiggly", 0, 0);
    glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	input_init(window);
	input_set_callbacks(window);

    if (hogl_init_gl_extensions() == -1) {
		printf("Could not initialize OpenGL extensions. Make sure you have OpenGL drivers 3.3 or latest\n");
		return EXIT_FAILURE;
	}

	Font_Load_Status status = font_load(OS_DEFAULT_FONT, &font_info, 16);

	hogui_init();

	HoGui_Window w = {
		.flags = HOGUI_WINDOW_FLAG_TOPDOWN|HOGUI_WINDOW_FLAG_CLIP_CHILDREN,
		.width = 100.0f,
		.height = 100.0f,
		.position = (vec2){100.0f, 100.0f},
		.bg_color = (vec4){1.0f, 0.0f, 0.0f, 1.0f},
	};
	HoGui_Window* m = hogui_new_window(&w, 0);

	HoGui_Window w2 = {
		.position = (vec2){20.0f, 0.0f},
		.width = 100.0f,
		.height = 20.0f,
		.bg_color = (vec4){0.0f, 1.0f, 0.0f, 1.0f},
	};
	HoGui_Window* m2 = hogui_new_window(&w2, m);

	HoGui_Window w2_2 = {
		.position = (vec2){20.0f, 0.0f},
		.width = 100.0f,
		.height = 20.0f,
		.bg_color = (vec4){0.0f, 1.0f, 1.0f, 1.0f},
	};
	HoGui_Window* m2_2 = hogui_new_window(&w2_2, m);

	//HoGui_Window w3 = {
	//	.width = 20.0f,
	//	.height = 20.0f,
	//	.bg_color = (vec4){0.0f, 0.0f, 1.0f, 1.0f},
	//};
	//HoGui_Window* m3 = hogui_new_window(&w3, m2);

	glClearColor(0.2f, 0.2f, 0.23f, 1.0f);

	bool running = true;
    while (!glfwWindowShouldClose(window) && running) {
		glfwPollEvents();

		Event e;
		while (event_pop(&e)) {
			if(e.type == EVENT_KEYBOARD_INPUT) {
				switch(e.keyboard.type) {
					case KEYBOARD_KEY_PRESS: {
						if(e.keyboard.unicode == GLFW_KEY_ESCAPE)
							running = false;
					}break;
					default: break;
				}
			}
		}

		int width, height;
		window_get_size(&width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		hogui_render();

		renderer_imm_enable_blending();
		renderer_imm_flush(font_info.atlas_full_id);
		renderer_imm_disable_blending();

        glfwSwapBuffers(window);
    }

	glfwTerminate();

    return 0;
}