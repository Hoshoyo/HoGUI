#define HOGL_IMPLEMENT
#define GRAPHICS_MATH_IMPLEMENT
#define USTRING_IMPLEMENT
#define LIGHT_ARENA_IMPLEMENT
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <ho_gl.h>
#include <light_array.h>
#include <gm.h>
#include "os.h"
#include "input.h"
#include "event.h"
#include "renderer/renderer_imm.h"
#include "renderer/font_load.h"
#include "hogui/hg_ui.h"
#include "hogui/hg_input.h"
#include "lexer.h"
#include "light_arena.h"

static r32 frame_times[120];
static int frame_times_index;

void hg_test(HG_Context* ctx) {
	static vec2 position;
	static HG_Container cont, cont2, cont3;
	static HG_Container_Column cols[2];
	cols[0].width = 0.5f;
	cols[1].width = 0.5f;

	int id = 1;
	hg_window_begin(ctx, id++, &position, 400, 100, "Foo");

	hg_do_container_column(ctx, id++, &cont, 1, 1, cols, 2, HG_CONTAINER_TOP_DOWN);
	hg_do_graph(ctx, id++, 0, frame_times, 120, 1000.0f / 120.0f, (vec4){1,1,1,1});
	//hg_do_container(ctx, id++, &cont, 1.0f, 1.0f, HG_CONTAINER_TOP_DOWN);
	for(s32 i = 0; i < 10; ++i) {
		char b[64] = {0};
		int l = sprintf(b, "foo %d", i);
		hg_do_label(ctx, id++, 0, b, l, (vec4){1,1,1,1});
		hg_column_next(ctx);
		hg_do_label(ctx, id++, 0, b, l, (vec4){1,1,1,1});
		hg_column_previous(ctx);
	}

	text_render_debug(&ctx->font_info, "{%f,%f,%f,%f} %f", 20, ctx->current_frame.x, ctx->current_frame.y, 
		ctx->current_frame.width, ctx->current_frame.height, cont.total_height);

	hg_do_container_end(ctx, &cont);
}

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

	r64 dt = 1.0/120.0;
	s32 frames = 0;
	r64 total_time = 0.0;
	r64 start_time = os_time_us();

	glClearColor(0.2f, 0.2f, 0.23f, 1.0f);

	HG_Context* gui_ctx = hg_init();

	bool running = true;

    while (!glfwWindowShouldClose(window) && running) {
		glfwPollEvents();
		int width, height;
		window_get_size(&width, &height);


		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		r64 start_render = os_time_us();
		{
			hg_start(gui_ctx);
			input_immgui(width, height);
			Event e = {0};
			while (event_pop(&e));

			hg_test(gui_ctx);

			hg_end(gui_ctx);
			hg_render(gui_ctx);
		}
		r64 end_render = os_time_us();

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

		r32 frame_time = os_time_us() - start_time;
		frame_times[frame_times_index++ % (sizeof(frame_times) / sizeof(*frame_times))] = (r32)((end_render - start_render) / 1000.0);

		total_time += frame_time;
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