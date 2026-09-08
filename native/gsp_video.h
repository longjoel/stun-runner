#ifndef STUNRUN_NATIVE_GSP_VIDEO_H
#define STUNRUN_NATIVE_GSP_VIDEO_H

#include <stddef.h>
#include <stdint.h>

#include "render.h"

typedef struct stunrun_gsp_video_state {
    uint16_t *vram_words;
    size_t vram_word_count;
    uint8_t palette_rgb[256u * 3u];
} stunrun_gsp_video_state_t;

/* Decode the two observed 256-entry palette planes into the RGB layout used
 * by render.c: R and G come from the low plane's high/low bytes, and B comes
 * from the high plane's low byte. */
int stunrun_gsp_palette_decode(const uint16_t *palette_low,
                               const uint16_t *palette_high,
                               uint8_t *palette_rgb,
                               size_t palette_rgb_size);

/* Install the observed two-plane palette into an initialized video state. */
int stunrun_gsp_video_set_palette_planes(stunrun_gsp_video_state_t *state,
                                         const uint16_t *palette_low,
                                         const uint16_t *palette_high);

/* Copy a captured VRAM word array into owned video state. */
int stunrun_gsp_video_set_vram_words(stunrun_gsp_video_state_t *state,
                                     const uint16_t *vram_words,
                                     size_t vram_word_count);
/* Install little-endian captured VRAM bytes into owned video state. */
int stunrun_gsp_video_set_vram_bytes(stunrun_gsp_video_state_t *state,
                                     const uint8_t *vram_bytes,
                                     size_t vram_byte_count);

void stunrun_gsp_video_init(stunrun_gsp_video_state_t *state);
void stunrun_gsp_video_free(stunrun_gsp_video_state_t *state);
int stunrun_gsp_video_load(stunrun_gsp_video_state_t *state,
                           const char *vram_path, const char *palette_path);
/* Load little-endian raw palette-plane snapshots without preconverting them
 * to RGB. Each palette file contains exactly 256 16-bit words. */
int stunrun_gsp_video_load_palette_planes(stunrun_gsp_video_state_t *state,
                                          const char *vram_path,
                                          const char *palette_low_path,
                                          const char *palette_high_path);
/* Render a loaded GSP state through the evidence-backed visible layout. */
int stunrun_gsp_video_render(const stunrun_gsp_video_state_t *state,
                             stunrun_renderer_t *renderer);

#endif
