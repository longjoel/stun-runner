#include "craft.h"

#include <stddef.h>

#define CRAFT_WHITE_RED 159u
#define CRAFT_WHITE_GREEN 153u
#define CRAFT_WHITE_BLUE 154u
#define CRAFT_RED_RED 191u
#define CRAFT_RED_GREEN 0u
#define CRAFT_RED_BLUE 0u
#define CRAFT_WING_RED 207u
#define CRAFT_WING_GREEN 0u
#define CRAFT_WING_BLUE 0u

static void set_quad(stunrun_road_strip_t *strip, float x0, float y0,
                     float x1, float y1, float x2, float y2, float x3,
                     float y3, unsigned red, unsigned green, unsigned blue)
{
    strip->vertices[0] = (stunrun_road_vertex_t){x0, y0, 0.0f};
    strip->vertices[1] = (stunrun_road_vertex_t){x1, y1, 0.0f};
    strip->vertices[2] = (stunrun_road_vertex_t){x2, y2, 0.0f};
    strip->vertices[3] = (stunrun_road_vertex_t){x3, y3, 0.0f};
    strip->red = (uint8_t)red;
    strip->green = (uint8_t)green;
    strip->blue = (uint8_t)blue;
}

void stunrun_craft_frame(stunrun_road_frame_t *frame)
{
    if (frame == NULL)
        return;
    frame->count = 0u;
    /* Panel-stripe band (array head = nearest: the frame renderer paints
     * strips back-to-front from the tail). Black trapezoid y167 top
     * x235-266 widening to y170 x231-277, measured dot runs from the
     * gated shot-1038. Edge remnants (2px) stay open. */
    set_quad(&frame->strips[frame->count++], 231.0f, 170.0f, 277.0f, 170.0f,
             266.0f, 167.0f, 235.0f, 167.0f, 0u, 0u, 0u);
    /* Wing stubs (paint first, behind the fuselage). Measured from
     * the gated shot-1038 red runs: left tip reaches x225, right tip x271
     * at y169; both taper to the fuselage sides at y159. Modal wing shade
     * is (207,0,0); the gradient and grey-speck overlap are open. */
    set_quad(&frame->strips[frame->count++], 225.0f, 169.0f, 234.0f, 169.0f,
             247.0f, 159.0f, 238.0f, 159.0f, CRAFT_WING_RED,
             CRAFT_WING_GREEN, CRAFT_WING_BLUE);
    set_quad(&frame->strips[frame->count++], 262.0f, 169.0f, 271.0f, 169.0f,
             258.0f, 159.0f, 249.0f, 159.0f, CRAFT_WING_RED,
             CRAFT_WING_GREEN, CRAFT_WING_BLUE);
    /* Lower red mass under the fuselage (array tail = farther). */
    set_quad(&frame->strips[frame->count++], 215.0f, 182.0f, 290.0f, 182.0f,
             274.0f, 172.0f, 228.0f, 172.0f, CRAFT_RED_RED, CRAFT_RED_GREEN,
             CRAFT_RED_BLUE);
    /* Fuselage core. */
    set_quad(&frame->strips[frame->count++], 228.0f, 172.0f, 274.0f, 172.0f,
             262.0f, 160.0f, 239.0f, 160.0f, CRAFT_RED_RED, CRAFT_RED_GREEN,
             CRAFT_RED_BLUE);
    /* Nose bridge. */
    set_quad(&frame->strips[frame->count++], 246.0f, 160.0f, 258.0f, 160.0f,
             260.0f, 154.0f, 244.0f, 154.0f, CRAFT_RED_RED, CRAFT_RED_GREEN,
             CRAFT_RED_BLUE);
    /* Canopy left lobe. */
    set_quad(&frame->strips[frame->count++], 239.0f, 158.0f, 243.0f, 158.0f,
             249.0f, 150.0f, 241.0f, 150.0f, CRAFT_WHITE_RED,
             CRAFT_WHITE_GREEN, CRAFT_WHITE_BLUE);
    /* Canopy right lobe. */
    set_quad(&frame->strips[frame->count++], 257.0f, 158.0f, 263.0f, 158.0f,
             261.0f, 150.0f, 254.0f, 150.0f, CRAFT_WHITE_RED,
             CRAFT_WHITE_GREEN, CRAFT_WHITE_BLUE);
    /* Canopy top (array tail = farthest; no overlaps, order benign). */
    set_quad(&frame->strips[frame->count++], 241.0f, 150.0f, 259.0f, 150.0f,
             256.0f, 147.0f, 243.0f, 147.0f, CRAFT_WHITE_RED,
             CRAFT_WHITE_GREEN, CRAFT_WHITE_BLUE);
}

int stunrun_render_craft(stunrun_renderer_t *renderer,
                         const stunrun_road_frame_t *frame)
{
    if (renderer == NULL || frame == NULL ||
        frame->count > STUNRUN_ROAD_FRAME_MAX_STRIPS)
        return 0;
    return stunrun_render_road_frame(renderer, frame);
}
