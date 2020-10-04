#include <stdio.h>
#define HOGL_IMPLEMENT
#include <ho_gl.h>
#define GRAPHICS_MATH_IMPLEMENT
#include "gm.h"
#include <GLFW/glfw3.h>

#include "hogui/hhu.h"
#include <time.h>
#if defined(__linux__)
#include <unistd.h>
#endif

void render()
{
    static HGui_Window hwindow;
    hhu_window(&hwindow);
}

int main()
{
    if (!glfwInit()) {
        printf("Could not initialize GLFW\n");
        return -1;
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
        return -1;
    }

    glClearColor(0.2f, 0.2f, 0.23f, 1.0f);

    hhu_init();
    hhu_glfw_init(window);

    int running = 1;

    while (!glfwWindowShouldClose(window) && running)
    {
        glfwPollEvents();
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        hhu_begin();
        render();
        hhu_end();

#if defined(__linux__)
        usleep(15000);
#endif

        glfwSwapBuffers(window);
    }

    glfwTerminate();

    return 0;
}