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
#include "renderer/font_load.h"
#include "hogui/hg_ui.h"
#include "hogui/hg_input.h"

static vec2 position;
void hg_test(HG_Context* ctx) {
	static HG_Container cont, cont2, cont3;
	static HG_Container_Column cols[2];
	cols[0].width = 0.5f;
	cols[1].width = 0.5f;

	int id = 1;
	hg_window_begin(ctx, id++, &position, 400, 300, "Foo");
	hg_do_container_column(ctx, id++, &cont2, 1.0f, 1.0f, cols, 2, HG_CONTAINER_TOP_DOWN);
	hg_do_container(ctx, id++, &cont, 1.0f, 100.0f, HG_CONTAINER_TOP_DOWN);
	hg_do_label(ctx, id++, 0, "Foo1SHDOASDHOASUDHOUAHSDHOAOSDHHAOSUDHAOSHDoHASUOHDSAOHDHASDASOHDHaos", 69, (vec4){1,1,1,1});
	hg_do_label(ctx, id++, 0, "Foo1", 4, (vec4){1,1,1,1});
	hg_do_label(ctx, id++, 0, "Foo2", 4, (vec4){1,1,1,1});
	hg_do_label(ctx, id++, 0, "Foo3", 4, (vec4){1,1,1,1});
	hg_do_label(ctx, id++, 0, "Foo4", 4, (vec4){1,1,1,1});
	hg_do_label(ctx, id++, 0, "Foo5", 4, (vec4){1,1,1,1});
	hg_do_label(ctx, id++, 0, "Foo6", 4, (vec4){1,1,1,1});
	hg_do_label(ctx, id++, 0, "Foo6", 4, (vec4){1,1,1,1});
	hg_do_label(ctx, id++, 0, "Foo6", 4, (vec4){1,1,1,1});
	hg_do_label(ctx, id++, 0, "Foo6", 4, (vec4){1,1,1,1});
	hg_do_container_end(ctx, &cont);
	hg_column_next(ctx);
	hg_do_container(ctx, id++, &cont3, 1.0f, 100.0f, HG_CONTAINER_TOP_DOWN);
	hg_do_label(ctx, id++, 0, "Foo8", 4, (vec4){1,1,1,1});
	hg_do_label(ctx, id++, 0, "Foo9", 4, (vec4){1,1,1,1});
	hg_do_label(ctx, id++, 0, "Foo9", 4, (vec4){1,1,1,1});
	hg_do_label(ctx, id++, 0, "Foo9", 4, (vec4){1,1,1,1});
	hg_do_label(ctx, id++, 0, "Foo9", 4, (vec4){1,1,1,1});
	hg_do_label(ctx, id++, 0, "Foo9", 4, (vec4){1,1,1,1});
	hg_do_container_end(ctx, &cont3);
	hg_do_container_end(ctx, &cont2);

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

		{
			hg_start(gui_ctx);
			input_immgui(width, height);
			Event e = {0};
			while (event_pop(&e));

			hg_test(gui_ctx);

			hg_end(gui_ctx);
			hg_render(gui_ctx);
		}

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