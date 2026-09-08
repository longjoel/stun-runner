/* Literal GSP PIXBLT text-cursor transport recovered at 0xFFF46590.
 *
 * This module decodes descriptor bytes only. It does not assign meaning to
 * the fields containing the descriptors, and the caller supplies the y bias
 * because the observed A1-to-screen mapping is checkpoint-specific.
 */
#ifndef STUNRUN_GSP_TEXT_CURSOR_H
#define STUNRUN_GSP_TEXT_CURSOR_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct stunrun_gsp_text_glyph {
    uint32_t descriptor_word_address;
    uint32_t a0_after;
    uint32_t a1;
    uint8_t lane;
    uint8_t glyph_code;
    int x;
    int y;
} stunrun_gsp_text_glyph_t;

/* Decode low-byte/high-byte lanes in order. initial_a0 is the address of the
 * first packed descriptor word; initial_a1 supplies the first cell. A0 moves
 * eight address units per glyph and A1 moves eight in its low halfword. */
size_t stunrun_gsp_text_cursor_decode(
    uint32_t initial_a0, uint32_t initial_a1, int y_bias,
    const uint16_t *packed_words, size_t packed_word_count,
    stunrun_gsp_text_glyph_t *out, size_t out_capacity);

#ifdef __cplusplus
}
#endif

#endif
