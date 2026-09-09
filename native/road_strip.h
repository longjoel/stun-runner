/* Backend-neutral projected road strip.
 *
 * The vertex coordinates are an explicit rendering fixture boundary. They are
 * not a claim about the unresolved encoding of the original road words.
 */
#ifndef STUNRUN_NATIVE_ROAD_STRIP_H
#define STUNRUN_NATIVE_ROAD_STRIP_H

#include <stdint.h>

#include "render.h"

typedef struct stunrun_road_vertex {
    float x;
    float y;
    float z;
} stunrun_road_vertex_t;

typedef struct stunrun_road_strip {
    stunrun_road_vertex_t vertices[4];
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} stunrun_road_strip_t;

/* Construct the first deterministic projected-strip fixture. */
void stunrun_road_strip_fixture(stunrun_road_strip_t *strip);

/* Software backend used for deterministic tests and image comparison. */
int stunrun_render_road_strip(stunrun_renderer_t *renderer,
                              const stunrun_road_strip_t *strip);

#endif
