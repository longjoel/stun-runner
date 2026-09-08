/* Deterministic software framebuffer boundary for the native shell.
 *
 * This module owns pixels and frame hashes only. It intentionally contains no
 * game drawing or display-semantic assumptions.
 */
#ifndef STUNRUN_NATIVE_RENDER_H
#define STUNRUN_NATIVE_RENDER_H

#include <stddef.h>
#include <stdint.h>

#define STUNRUN_RENDER_WIDTH 512u
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
int stunrun_render_blit(stunrun_renderer_t *renderer, const uint8_t *source,
                        unsigned source_width, unsigned source_height,
                        int destination_x, int destination_y);
/* Evidence-backed GSP PIXBLT candidate: four little-endian 16-bit words hold
 * an 8x8 one-bit tile, with pixels consumed least-significant-bit first. A
 * zero source bit is transparent; a one bit receives the supplied color. */
int stunrun_render_gsp_glyph_8x8(stunrun_renderer_t *renderer,
                                 const uint16_t source_words[4],
                                 int destination_x, int destination_y,
                                 uint8_t red, uint8_t green, uint8_t blue);
/* Select one tile using the traced source = base + (code & 0x7f) * 64
 * address formula. The table contains four 16-bit words per tile. */
int stunrun_render_gsp_glyph_from_table(stunrun_renderer_t *renderer,
                                        const uint16_t *source_table,
                                        size_t source_word_count,
                                        unsigned glyph_code,
                                        int destination_x, int destination_y,
                                        uint8_t red, uint8_t green,
                                        uint8_t blue);
/* Render an explicit glyph-code run using the observed 8-pixel cell advance.
 * This accepts already selected codes; it does not decode GSP record lanes. */
int stunrun_render_gsp_text_8x8(stunrun_renderer_t *renderer,
                                const uint16_t *source_table,
                                size_t source_word_count,
                                const uint8_t *glyph_codes,
                                size_t glyph_count,
                                int destination_x, int destination_y,
                                uint8_t red, uint8_t green, uint8_t blue);
/* Expand the captured packed-text word order (low byte, then high byte) and
 * stop at the first NUL byte. */
int stunrun_render_gsp_packed_text_8x8(stunrun_renderer_t *renderer,
                                       const uint16_t *source_table,
                                       size_t source_word_count,
                                       const uint16_t *packed_words,
                                       size_t packed_word_count,
                                       int destination_x, int destination_y,
                                       uint8_t red, uint8_t green,
                                       uint8_t blue);
/* Clipped inclusive-coordinate fill counterpart for the traced GSP FILL XY
 * operation. Bounds may be supplied in either order; this is a raster
 * primitive only and assigns no meaning to the coordinates. */
int stunrun_render_fill_xy(stunrun_renderer_t *renderer, int x0, int y0,
                           int x1, int y1, uint8_t red, uint8_t green,
                           uint8_t blue);
/* Clipped line counterpart for the traced GSP LINE 0 primitive. */
int stunrun_render_line(stunrun_renderer_t *renderer, int x0, int y0,
                        int x1, int y1, uint8_t red, uint8_t green,
                        uint8_t blue);
/* Render the evidence-backed visible multisync layout: four 512-byte lines
 * per 2048-byte VRAM row, with little-endian 16-bit words and RGB palette. */
int stunrun_render_gsp_visible(stunrun_renderer_t *renderer,
                               const uint16_t *vram_words,
                               size_t vram_word_count,
                               const uint8_t *palette_rgb);
uint32_t stunrun_render_hash(const stunrun_renderer_t *renderer);
int stunrun_render_write_ppm(const stunrun_renderer_t *renderer,
                             const char *path);

#endif
