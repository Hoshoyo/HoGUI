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
#include "hogui/immgui_input.h"
#include "hogui/immgui.h"

Font_Info font_info = {0};

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

	r64 dt = 1.0/120.0;
	s32 frames = 0;
	r64 total_time = 0.0;
	r64 start_time = os_time_us();

	glClearColor(0.2f, 0.2f, 0.23f, 1.0f);

	HG_Context ctx = {0};
	ctx.active.owner = -1;
	ctx.hot.owner = -1;
	ctx.last_hot = -1;

	bool running = true;
	char buffer[3][256];
	int length[3] = {0};
	int cursor_index[3] = {0};
	int selection_distance[3] = {0};

	vec2 w1_pos = (vec2){0,0};
	vec2 w2_pos = (vec2){300,200};

	r32 value = 5.0f;

    while (!glfwWindowShouldClose(window) && running) {
		glfwPollEvents();
		int width, height;
		window_get_size(&width, &height);

		input_immgui(window);
		input_immgui_set_window_size(width, height);

		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		{
			hg_end(&ctx);
			hg_start(&ctx);

			hg_window_begin(&ctx, 200, &w1_pos, 500, 500, "Foo", 1);
			if(hg_do_button(&ctx, 100, "Hello", sizeof("Hello") - 1)) {
				printf("hello\n");
			}
			hg_window_begin(&ctx, 201, &w2_pos, 300, 200, "Foo", 1);
			if(hg_do_button(&ctx, 101, "World", sizeof("World") - 1)) {
				printf("world\n");
			}
			hg_do_slider(&ctx, 300, &value, 0, 10);
		}
	
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
			printf("Rendered %d frames per second\n", frames);
			total_time = 0;
			frames = 0;
		}

		start_time = os_time_us();

        glfwSwapBuffers(window);
    }

	glfwTerminate();

    return 0;
}