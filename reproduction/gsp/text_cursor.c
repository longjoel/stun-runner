/* Literal GSP PIXBLT text cursor slice. */

#include "text_cursor.h"

#include <limits.h>

size_t stunrun_gsp_text_cursor_decode(
    uint32_t initial_a0, uint32_t initial_a1, int y_bias,
    const uint16_t *packed_words, size_t packed_word_count,
    stunrun_gsp_text_glyph_t *out, size_t out_capacity)
{
    size_t word_index;
    size_t glyph_index = 0u;
    int initial_x = (int)(initial_a1 & 0xffffu);
    int initial_y = (int)((initial_a1 >> 16) & 0xffffu) - y_bias;

    if (packed_words == NULL || out == NULL || out_capacity == 0u)
        return 0u;
    for (word_index = 0u; word_index < packed_word_count; word_index++) {
        unsigned lane;
        for (lane = 0u; lane < 2u; lane++) {
            uint8_t code = (uint8_t)((packed_words[word_index] >> (lane * 8u)) & 0xffu);
            int x;
            if (code == 0u)
                return glyph_index;
            if (glyph_index >= out_capacity || glyph_index > (size_t)INT_MAX / 8u)
                return 0u;
            x = initial_x + (int)(glyph_index * 8u);
            out[glyph_index].descriptor_word_address =
                initial_a0 + (uint32_t)((glyph_index / 2u) * 16u);
            out[glyph_index].a0_after =
                initial_a0 + (uint32_t)((glyph_index + 1u) * 8u);
            out[glyph_index].a1 =
                (initial_a1 & 0xffff0000u) |
                (uint32_t)((initial_x + (int)(glyph_index * 8u)) & 0xffff);
            out[glyph_index].lane = (uint8_t)lane;
            out[glyph_index].glyph_code = code;
            out[glyph_index].x = x;
            out[glyph_index].y = initial_y;
            glyph_index++;
        }
    }
    return glyph_index;
}
