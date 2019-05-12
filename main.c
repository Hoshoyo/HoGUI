#define HOGL_IMPLEMENT
#define GRAPHICS_MATH_IMPLEMENT
#define USTRING_IMPLEMENT
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <ho_gl.h>
#include <light_array.h>
#include <gm.h>
#include "os.h"
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
	if(status != FONT_LOAD_OK) {
		printf("Could not load font %s\n", OS_DEFAULT_FONT);
		return 1;
	}

	hogui_init();

	HoGui_Window w = {
		.flags = HOGUI_WINDOW_FLAG_TOPDOWN|HOGUI_WINDOW_FLAG_CLIP_CHILDREN,
		.width = 1024.0f,
		.height = 768.0f,
		.position = (vec2){100.0f, 100.0f},
		.bg_color = (vec4){1.0f, 0.0f, 0.0f, 1.0f},
	};
	HoGui_Window* m = hogui_new_window(&w, 0);

	HoGui_Window** ws = array_new(HoGui_Window*);
	for(int i = 0; i < 10; ++i) {		
		HoGui_Window ww = {
			.flags = HOGUI_WINDOW_FLAG_TOPDOWN|HOGUI_WINDOW_FLAG_CLIP_CHILDREN,
			.position = (vec2){20.0f, 10.0f},
			.width = 200.0f,
			.height = 40.0f,
			.bg_color = (vec4){0.0f, 1.0f, 0.0f, 1.0f},
		};
		HoGui_Window* mm = hogui_new_window(&ww, m);
		array_push(ws, mm);
	}

	//HoGui_Window w3 = {
	//	.width = 2000.0f,
	//	.height = 2000.0f,
	//	.bg_color = (vec4){0.0f, 0.0f, 1.0f, 1.0f},
	//};
	//HoGui_Window* m3 = hogui_new_window(&w3, m2);

	r64 dt = 1.0/60.0;
	s32 frames = 0;
	r64 total_time = 0.0;
	r64 start_time = os_time_us();

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
						if(e.keyboard.unicode == 'X') {
							if(array_length(ws) > 0) {
								hogui_delete_window(ws[array_length(ws) - 1]);
								array_pop(ws);
							}
						}
						if(e.keyboard.unicode == 'A') {
							HoGui_Window ww = {
								.flags = HOGUI_WINDOW_FLAG_TOPDOWN|HOGUI_WINDOW_FLAG_CLIP_CHILDREN,
								.position = (vec2){20.0f, 10.0f},
								.width = 200.0f,
								.height = 40.0f,
								.bg_color = (vec4){0.0f, 1.0f, 0.0f, 1.0f},
							};
							HoGui_Window* mm = hogui_new_window(&ww, m);
							array_push(ws, mm);
						}
					}break;
					default: break;
				}
			}
		}

		int width, height;
		window_get_size(&width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		hogui_render(&font_info);

		renderer_imm_enable_blending();
		glDisable(GL_DEPTH_TEST);
		renderer_imm_flush(font_info.atlas_full_id);
		renderer_imm_disable_blending();


		// Calculate elapsed time
		// TODO(psv): make an OS level function here instead of ifdefs
		r64 elapsed_us = os_time_us() - start_time;
		r64 sleep_time = (dt * 1000000.0) - elapsed_us;
#if defined(__linux__)
		if (sleep_time > 0.0) {
			os_usleep((u64)sleep_time);
		}
#endif
		frames++;

		total_time += os_time_us() - start_time;
		if (total_time > 1000000.0) {
			//logger_log_info("Rendered %d frames per second", frames);
			total_time = 0;
			frames = 0;
		}

		start_time = os_time_us();

        glfwSwapBuffers(window);
    }

	glfwTerminate();

    return 0;
}