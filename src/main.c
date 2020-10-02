#include <stdio.h>
#define HOGL_IMPLEMENT
#include <ho_gl.h>
#define GRAPHICS_MATH_IMPLEMENT
#include <gm.h>
#include <GLFW/glfw3.h>

#include "common.h"
#include "hhu.h"
#include "input.h"

int main()
{
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

    if (hogl_init_gl_extensions() == -1) {
        printf("Could not initialize OpenGL extensions. Make sure you have OpenGL drivers 3.3 or latest\n");
        return EXIT_FAILURE;
    }

    glClearColor(0.2f, 0.2f, 0.23f, 1.0f);

    HHU_Context* hhu_ctx = hhu_init();
    hhu_init_glfw(hhu_ctx, window);
    //hhu_input_init(hhu_ctx);
    hinp_init(window);

    bool running = true;

    while (!glfwWindowShouldClose(window) && running)
    {
        //hhu_input_clear(hhu_ctx);
        glfwPollEvents();
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        hhu_render(hhu_ctx);

        glfwSwapBuffers(window);
    }

    glfwTerminate();

    return 0;
}