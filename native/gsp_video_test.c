#include "gsp_video.h"

#include <stdio.h>

int main(void)
{
    stunrun_gsp_video_state_t state;
    stunrun_gsp_video_init(&state);
    if (state.vram_words != NULL || state.vram_word_count != 0u ||
        state.palette_rgb[0] != 0u) {
        puts("gsp video init failed");
        return 1;
    }
    stunrun_gsp_video_free(&state);
    {
        uint16_t low[256] = {0};
        uint16_t high[256] = {0};
        uint8_t rgb[256u * 3u] = {0};
        low[7] = 0x1234u;
        high[7] = 0xABCDu;
        if (!stunrun_gsp_palette_decode(low, high, rgb, sizeof(rgb)) ||
            rgb[7u * 3u] != 0x12u || rgb[7u * 3u + 1u] != 0x34u ||
            rgb[7u * 3u + 2u] != 0xCDu ||
            stunrun_gsp_palette_decode(low, high, rgb, sizeof(rgb) - 1u) ||
            stunrun_gsp_palette_decode(NULL, high, rgb, sizeof(rgb))) {
            puts("gsp palette decode failed");
            return 1;
        }
        stunrun_gsp_video_init(&state);
        if (!stunrun_gsp_video_set_palette_planes(&state, low, high) ||
            state.palette_rgb[7u * 3u] != 0x12u ||
            state.palette_rgb[7u * 3u + 1u] != 0x34u ||
            state.palette_rgb[7u * 3u + 2u] != 0xCDu ||
            stunrun_gsp_video_set_palette_planes(&state, NULL, high)) {
            puts("gsp video palette state failed");
            return 1;
        }
        if (state.palette_rgb[7u * 3u] != 0x12u ||
            state.palette_rgb[7u * 3u + 1u] != 0x34u ||
            state.palette_rgb[7u * 3u + 2u] != 0xCDu) {
            puts("gsp video palette transaction failed");
            return 1;
        }
    }
    {
        uint16_t vram[4] = {0x0201u, 0u, 0u, 0u};
        stunrun_renderer_t renderer;
        if (!stunrun_gsp_video_set_vram_words(&state, vram, 4u) ||
            state.vram_words == vram || state.vram_words[0] != 0x0201u ||
            stunrun_gsp_video_set_vram_words(&state, vram, 3u) ||
            stunrun_gsp_video_set_vram_words(&state, NULL, 4u)) {
            puts("gsp video VRAM state failed");
            return 1;
        }
        if (state.vram_words == NULL || state.vram_word_count != 4u ||
            state.vram_words[0] != 0x0201u) {
            puts("gsp video VRAM transaction failed");
            return 1;
        }
        {
            const uint8_t vram_bytes[] = {0x34u, 0x12u, 0xCDu, 0xABu,
                                          0u, 0u, 0u, 0u};
            if (!stunrun_gsp_video_set_vram_bytes(
                    &state, vram_bytes, sizeof(vram_bytes)) ||
                state.vram_word_count != 4u || state.vram_words[0] != 0x1234u ||
                state.vram_words[1] != 0xABCDu ||
                stunrun_gsp_video_set_vram_bytes(&state, vram_bytes, 6u) ||
                state.vram_words[0] != 0x1234u) {
                puts("gsp video byte VRAM state failed");
                return 1;
            }
        }
        if (!stunrun_gsp_video_set_vram_words(&state, vram, 4u)) {
            puts("gsp video word VRAM restore failed");
            return 1;
        }
        state.palette_rgb[1u * 3u] = 0x11u;
        state.palette_rgb[2u * 3u] = 0x22u;
        stunrun_render_init(&renderer);
        if (!stunrun_gsp_video_render(&state, &renderer) ||
            renderer.pixels[0] != 0x11u || renderer.pixels[3] != 0x22u ||
            stunrun_gsp_video_render(NULL, &renderer)) {
            puts("gsp video render integration failed");
            return 1;
        }
        stunrun_gsp_video_free(&state);
    }
    puts("native gsp video boundary: all checks passed");
    return 0;
}
