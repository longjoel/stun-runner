#include "opengl_backend.h"

#include <GL/gl.h>

void stunrun_opengl_draw_road_strip(const stunrun_road_strip_t *strip)
{
    unsigned i;
    if (strip == NULL)
        return;
    glColor3ub(strip->red, strip->green, strip->blue);
    glBegin(GL_TRIANGLE_STRIP);
    for (i = 0u; i < 4u; i++) {
        glVertex3f(strip->vertices[i].x, strip->vertices[i].y,
                   strip->vertices[i].z);
    }
    glEnd();
}
