#include "game_loop.h"
#include "opengl_backend.h"
#include "road_strip.h"

#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned frame_limit(void)
{
    const char *text = getenv("STUNRUN_OPENGL_FRAMES");
    char *end = NULL;
    unsigned long value;
    if (text == NULL || text[0] == '\0')
        return 0u;
    value = strtoul(text, &end, 10);
    return end != text && *end == '\0' && value <= 1000000ul ?
           (unsigned)value : 0u;
}

int main(void)
{
    GLFWwindow *window;
    stunrun_fake_ports_t ports;
    stunrun_game_loop_t loop;
    stunrun_road_strip_t strip;
    unsigned limit = frame_limit();
    unsigned frame = 0u;

    if (!glfwInit()) {
        fprintf(stderr, "opengl-demo: glfw initialization failed\n");
        return 2;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    window = glfwCreateWindow(512, 240, "S.T.U.N. Runner native renderer",
                              NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "opengl-demo: window creation failed\n");
        glfwTerminate();
        return 2;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glViewport(0, 0, 512, 240);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 512.0, 240.0, 0.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    stunrun_ports_init(&ports);
    stunrun_game_loop_init(&loop, &ports);
    stunrun_road_strip_fixture(&strip);
    while (!glfwWindowShouldClose(window) &&
           (limit == 0u || frame < limit)) {
        glfwPollEvents();
        stunrun_game_loop_step(&loop);
        glClearColor(0.02f, 0.03f, 0.04f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glLoadIdentity();
        stunrun_opengl_draw_road_strip(&strip);
        glfwSwapBuffers(window);
        frame++;
    }
    printf("opengl-demo: frames=%u ticks=%u\n", frame,
           (unsigned)loop.global_tick);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
