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

#define STUNRUN_ROAD_FRAME_MAX_STRIPS 16u

typedef struct stunrun_road_frame {
    stunrun_road_strip_t strips[STUNRUN_ROAD_FRAME_MAX_STRIPS];
    unsigned count;
} stunrun_road_frame_t;

/* Construct the first deterministic projected-strip fixture. */
void stunrun_road_strip_fixture(stunrun_road_strip_t *strip);

/* Construct an ordered multi-strip projection sandbox. */
void stunrun_road_frame_fixture(stunrun_road_frame_t *frame);

/* Software backend used for deterministic tests and image comparison. */
int stunrun_render_road_strip(stunrun_renderer_t *renderer,
                              const stunrun_road_strip_t *strip);

int stunrun_render_road_frame(stunrun_renderer_t *renderer,
                              const stunrun_road_frame_t *frame);

#endif
