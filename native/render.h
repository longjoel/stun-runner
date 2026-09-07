/* Deterministic software framebuffer boundary for the native shell.
 *
 * This module owns pixels and frame hashes only. It intentionally contains no
 * game drawing or display-semantic assumptions.
 */
#ifndef STUNRUN_NATIVE_RENDER_H
#define STUNRUN_NATIVE_RENDER_H

#include <stddef.h>
#include <stdint.h>

#define STUNRUN_RENDER_WIDTH 320u
#define STUNRUN_RENDER_HEIGHT 240u
#define STUNRUN_RENDER_CHANNELS 3u
#define STUNRUN_RENDER_BYTES \
    (STUNRUN_RENDER_WIDTH * STUNRUN_RENDER_HEIGHT * STUNRUN_RENDER_CHANNELS)

typedef struct stunrun_renderer {
    uint8_t pixels[STUNRUN_RENDER_BYTES];
    unsigned frame;
} stunrun_renderer_t;

void stunrun_render_init(stunrun_renderer_t *renderer);
void stunrun_render_begin(stunrun_renderer_t *renderer, unsigned frame,
                          uint8_t red, uint8_t green, uint8_t blue);
int stunrun_render_set_pixel(stunrun_renderer_t *renderer, unsigned x,
                             unsigned y, uint8_t red, uint8_t green,
                             uint8_t blue);
uint32_t stunrun_render_hash(const stunrun_renderer_t *renderer);
int stunrun_render_write_ppm(const stunrun_renderer_t *renderer,
                             const char *path);

#endif
