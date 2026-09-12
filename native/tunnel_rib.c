#include "tunnel_rib.h"

#include <stddef.h>

#define RIB_RED 151u
#define RIB_GREEN 43u
#define RIB_BLUE 34u

static void set_quad(stunrun_road_strip_t *strip, float x0, float y0,
                     float x1, float y1, float x2, float y2, float x3,
                     float y3)
{
    strip->vertices[0] = (stunrun_road_vertex_t){x0, y0, 0.0f};
    strip->vertices[1] = (stunrun_road_vertex_t){x1, y1, 0.0f};
    strip->vertices[2] = (stunrun_road_vertex_t){x2, y2, 0.0f};
    strip->vertices[3] = (stunrun_road_vertex_t){x3, y3, 0.0f};
    strip->red = RIB_RED;
    strip->green = RIB_GREEN;
    strip->blue = RIB_BLUE;
}

void stunrun_tunnel_rib_frame(stunrun_road_frame_t *frame)
{
    if (frame == NULL)
        return;
    frame->count = 0u;
    /* Array head = nearest (the frame renderer paints back-to-front from
     * the tail): near (lower) pair first, far (upper) pair at the tail. */
    /* Left lower facet: kink pair to pinched base. */
    set_quad(&frame->strips[frame->count++], 202.0f, 123.0f, 236.0f, 123.0f,
             228.0f, 135.0f, 227.0f, 135.0f);
    /* Right lower facet. */
    set_quad(&frame->strips[frame->count++], 262.0f, 123.0f, 291.0f, 123.0f,
             266.0f, 135.0f, 265.0f, 135.0f);
    /* Left upper facet: outer-top, inner-top, inner-kink, outer-kink. */
    set_quad(&frame->strips[frame->count++], 216.0f, 114.0f, 218.0f, 114.0f,
             236.0f, 123.0f, 202.0f, 123.0f);
    /* Right upper facet. */
    set_quad(&frame->strips[frame->count++], 281.0f, 113.0f, 281.0f, 113.0f,
             291.0f, 123.0f, 262.0f, 123.0f);
}

int stunrun_render_tunnel_rib(stunrun_renderer_t *renderer,
                              const stunrun_road_frame_t *frame)
{
    if (renderer == NULL || frame == NULL ||
        frame->count > STUNRUN_ROAD_FRAME_MAX_STRIPS)
        return 0;
    return stunrun_render_road_frame(renderer, frame);
}
