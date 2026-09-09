/* OpenGL sink for the backend-neutral projected road strip. */
#ifndef STUNRUN_NATIVE_OPENGL_BACKEND_H
#define STUNRUN_NATIVE_OPENGL_BACKEND_H

#include "road_strip.h"

/* Requires a current OpenGL context owned by the caller. */
void stunrun_opengl_draw_road_strip(const stunrun_road_strip_t *strip);
void stunrun_opengl_draw_road_frame(const stunrun_road_frame_t *frame);

#endif
