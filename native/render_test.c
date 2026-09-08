#include "render.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    stunrun_renderer_t renderer;
    stunrun_render_init(&renderer);
    stunrun_render_begin(&renderer, 17u, 0u, 0u, 0u);
    if (renderer.frame != 17u || stunrun_render_hash(&renderer) !=
        0x7d611dc5u)
        return 1;
    if (!stunrun_render_set_pixel(&renderer, 319u, 239u, 1u, 2u, 3u) ||
        stunrun_render_set_pixel(&renderer, 512u, 0u, 1u, 2u, 3u) ||
        stunrun_render_set_pixel(&renderer, 0u, 240u, 1u, 2u, 3u))
        return 1;
    if (stunrun_render_hash(&renderer) == 0x7d611dc5u)
        return 1;
    {
        static const uint8_t source[] = {
            1u, 2u, 3u, 4u, 5u, 6u,
            7u, 8u, 9u, 10u, 11u, 12u
        };
        size_t offset = ((size_t)239u * STUNRUN_RENDER_WIDTH + 511u) * 3u;
        if (!stunrun_render_blit(&renderer, source, 2u, 2u, 511, 239) ||
            memcmp(renderer.pixels + offset, source, 3u) != 0 ||
            !stunrun_render_blit(&renderer, source, 2u, 2u, -1, -1))
            return 1;
    }
    {
        /* Character '0' captured from the inferred 0xFFF5DBC0 8x8 source
         * table. Its LSB-first rows are 1C, 36, 63, 63, 63, 36, 1C, 00. */
        static const uint16_t glyph_zero[4] = {
            0x361Cu, 0x6363u, 0x3663u, 0x001Cu
        };
        if (!stunrun_render_gsp_glyph_8x8(&renderer, glyph_zero, 64, 213,
                                          0xFFu, 0xFEu, 0u) ||
            renderer.pixels[((size_t)213u * STUNRUN_RENDER_WIDTH + 66u) * 3u] !=
                0xFFu ||
            renderer.pixels[((size_t)214u * STUNRUN_RENDER_WIDTH + 65u) * 3u + 1u] !=
                0xFEu ||
            renderer.pixels[((size_t)215u * STUNRUN_RENDER_WIDTH + 64u) * 3u] !=
                0xFFu ||
            renderer.pixels[((size_t)213u * STUNRUN_RENDER_WIDTH + 64u) * 3u] !=
                0u ||
            renderer.pixels[((size_t)220u * STUNRUN_RENDER_WIDTH + 66u) * 3u] !=
                0u)
            return 1;
    }
    {
        /* Character 'C' at source-table index 0x43, independently matched
         * against the oracle's Credits label at (212,224). */
        static const uint16_t glyph_c[4] = {
            0x633Eu, 0x0303u, 0x6303u, 0x003Eu
        };
        if (!stunrun_render_gsp_glyph_8x8(&renderer, glyph_c, 212, 224,
                                          0xFFu, 0xFEu, 0u) ||
            renderer.pixels[((size_t)224u * STUNRUN_RENDER_WIDTH + 213u) * 3u] !=
                0xFFu ||
            renderer.pixels[((size_t)225u * STUNRUN_RENDER_WIDTH + 212u) * 3u + 1u] !=
                0xFEu ||
            renderer.pixels[((size_t)227u * STUNRUN_RENDER_WIDTH + 213u) * 3u] !=
                0xFFu ||
            renderer.pixels[((size_t)224u * STUNRUN_RENDER_WIDTH + 212u) * 3u] !=
                0u ||
            renderer.pixels[((size_t)231u * STUNRUN_RENDER_WIDTH + 213u) * 3u] !=
                0u)
            return 1;
    }
    {
        uint16_t source_table[128u * 4u] = {0};
        source_table[0x43u * 4u] = 0x633Eu;
        source_table[0x43u * 4u + 1u] = 0x0303u;
        source_table[0x43u * 4u + 2u] = 0x6303u;
        source_table[0x43u * 4u + 3u] = 0x003Eu;
        if (!stunrun_render_gsp_glyph_from_table(
                &renderer, source_table, sizeof(source_table) / sizeof(*source_table),
                0xC3u, 300, 100, 4u, 5u, 6u) ||
            renderer.pixels[((size_t)100u * STUNRUN_RENDER_WIDTH + 301u) * 3u] !=
                4u ||
            renderer.pixels[((size_t)101u * STUNRUN_RENDER_WIDTH + 300u) * 3u + 1u] !=
                5u ||
            renderer.pixels[((size_t)103u * STUNRUN_RENDER_WIDTH + 302u) * 3u] !=
                0u)
            return 1;
    }
    {
        uint16_t source_table[128u * 4u] = {0};
        static const uint8_t text[] = {0x43u, 0x72u};
        source_table[0x43u * 4u] = 0x633Eu;
        source_table[0x43u * 4u + 1u] = 0x0303u;
        source_table[0x43u * 4u + 2u] = 0x6303u;
        source_table[0x43u * 4u + 3u] = 0x003Eu;
        source_table[0x72u * 4u + 1u] = 0x6E3Eu;
        source_table[0x72u * 4u + 2u] = 0x0606u;
        source_table[0x72u * 4u + 3u] = 0x0006u;
        if (!stunrun_render_gsp_text_8x8(
                &renderer, source_table, sizeof(source_table) / sizeof(*source_table),
                text, sizeof(text), 212, 224, 0xFFu, 0xFEu, 0u) ||
            renderer.pixels[((size_t)224u * STUNRUN_RENDER_WIDTH + 213u) * 3u] !=
                0xFFu ||
            renderer.pixels[((size_t)226u * STUNRUN_RENDER_WIDTH + 220u) * 3u] !=
                0u ||
            renderer.pixels[((size_t)226u * STUNRUN_RENDER_WIDTH + 221u) * 3u] !=
                0xFFu)
            return 1;
    }
    if (!stunrun_render_fill_xy(&renderer, 4, 6, 2, 3, 0xA1u, 0xB2u,
                                0xC3u) ||
        renderer.pixels[((size_t)3u * STUNRUN_RENDER_WIDTH + 2u) * 3u] !=
            0xA1u ||
        renderer.pixels[((size_t)6u * STUNRUN_RENDER_WIDTH + 4u) * 3u + 1u] !=
            0xB2u ||
        renderer.pixels[((size_t)7u * STUNRUN_RENDER_WIDTH + 2u) * 3u] ==
            0xA1u ||
        !stunrun_render_fill_xy(&renderer, -4, -3, -1, -1, 1u, 2u, 3u))
        return 1;
    if (!stunrun_render_line(&renderer, 1, 1, 4, 4, 0xD1u, 0xE2u,
                             0xF3u) ||
        renderer.pixels[((size_t)1u * STUNRUN_RENDER_WIDTH + 1u) * 3u] !=
            0xD1u ||
        renderer.pixels[((size_t)3u * STUNRUN_RENDER_WIDTH + 3u) * 3u + 1u] !=
            0xE2u ||
        !stunrun_render_line(&renderer, -2, 0, 2, 0, 9u, 8u, 7u) ||
        renderer.pixels[2u * 3u] != 9u ||
        !stunrun_render_line(&renderer, 0, 0, 0, 0, 1u, 2u, 3u))
        return 1;
    {
        uint16_t vram[4] = {0x0201u, 0u, 0u, 0u};
        uint8_t palette[256u * 3u] = {0};
        palette[1u * 3u] = 0x12u;
        palette[1u * 3u + 1u] = 0x34u;
        palette[2u * 3u] = 0x56u;
        palette[2u * 3u + 1u] = 0x78u;
        if (!stunrun_render_gsp_visible(&renderer, vram, 4u, palette) ||
            renderer.pixels[0] != 0x12u || renderer.pixels[1] != 0x34u ||
            renderer.pixels[3] != 0x56u || renderer.pixels[4] != 0x78u)
            return 1;
    }
    puts("native render boundary: all checks passed");
    return 0;
}
