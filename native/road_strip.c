#include "road_strip.h"

#include <math.h>

static float edge(float ax, float ay, float bx, float by,
                  float px, float py)
{
    return (px - ax) * (by - ay) - (py - ay) * (bx - ax);
}

static int draw_triangle(stunrun_renderer_t *renderer,
                         const stunrun_road_strip_t *strip,
                         unsigned a, unsigned b, unsigned c)
{
    const stunrun_road_vertex_t *v0 = &strip->vertices[a];
    const stunrun_road_vertex_t *v1 = &strip->vertices[b];
    const stunrun_road_vertex_t *v2 = &strip->vertices[c];
    float area = edge(v0->x, v0->y, v1->x, v1->y, v2->x, v2->y);
    int min_x, max_x, min_y, max_y, x, y;

    if (area == 0.0f)
        return 1;
    min_x = (int)floorf(fminf(v0->x, fminf(v1->x, v2->x)));
    max_x = (int)ceilf(fmaxf(v0->x, fmaxf(v1->x, v2->x)));
    min_y = (int)floorf(fminf(v0->y, fminf(v1->y, v2->y)));
    max_y = (int)ceilf(fmaxf(v0->y, fmaxf(v1->y, v2->y)));
    if (min_x < 0) min_x = 0;
    if (min_y < 0) min_y = 0;
    if (max_x >= (int)STUNRUN_RENDER_WIDTH)
        max_x = (int)STUNRUN_RENDER_WIDTH - 1;
    if (max_y >= (int)STUNRUN_RENDER_HEIGHT)
        max_y = (int)STUNRUN_RENDER_HEIGHT - 1;
    for (y = min_y; y <= max_y; y++) {
        for (x = min_x; x <= max_x; x++) {
            float px = (float)x + 0.5f;
            float py = (float)y + 0.5f;
            float w0 = edge(v1->x, v1->y, v2->x, v2->y, px, py);
            float w1 = edge(v2->x, v2->y, v0->x, v0->y, px, py);
            float w2 = edge(v0->x, v0->y, v1->x, v1->y, px, py);
            int inside = area > 0.0f ?
                (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) :
                (w0 <= 0.0f && w1 <= 0.0f && w2 <= 0.0f);
            if (inside)
                (void)stunrun_render_set_pixel(renderer, (unsigned)x,
                                                (unsigned)y, strip->red,
                                                strip->green, strip->blue);
        }
    }
    return 1;
}

void stunrun_road_strip_fixture(stunrun_road_strip_t *strip)
{
    if (strip == NULL)
        return;
    strip->vertices[0] = (stunrun_road_vertex_t){120.0f, 210.0f, 0.0f};
    strip->vertices[1] = (stunrun_road_vertex_t){392.0f, 210.0f, 0.0f};
    strip->vertices[2] = (stunrun_road_vertex_t){272.0f, 110.0f, 1.0f};
    strip->vertices[3] = (stunrun_road_vertex_t){240.0f, 110.0f, 1.0f};
    strip->red = 0x40u;
    strip->green = 0x48u;
    strip->blue = 0x50u;
}

int stunrun_render_road_strip(stunrun_renderer_t *renderer,
                              const stunrun_road_strip_t *strip)
{
    if (renderer == NULL || strip == NULL)
        return 0;
    return draw_triangle(renderer, strip, 0u, 1u, 2u) &&
           draw_triangle(renderer, strip, 0u, 2u, 3u);
}
