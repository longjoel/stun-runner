#include "gsp_video.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int power_of_two(size_t value);

int stunrun_gsp_palette_decode(const uint16_t *palette_low,
                               const uint16_t *palette_high,
                               uint8_t *palette_rgb,
                               size_t palette_rgb_size)
{
    size_t index;

    if (palette_low == NULL || palette_high == NULL || palette_rgb == NULL ||
        palette_rgb_size < 256u * 3u)
        return 0;
    for (index = 0; index < 256u; index++) {
        palette_rgb[index * 3u] = (uint8_t)(palette_low[index] >> 8);
        palette_rgb[index * 3u + 1u] = (uint8_t)palette_low[index];
        palette_rgb[index * 3u + 2u] = (uint8_t)palette_high[index];
    }
    return 1;
}

int stunrun_gsp_video_set_palette_planes(stunrun_gsp_video_state_t *state,
                                         const uint16_t *palette_low,
                                         const uint16_t *palette_high)
{
    if (state == NULL)
        return 0;
    return stunrun_gsp_palette_decode(palette_low, palette_high,
                                      state->palette_rgb,
                                      sizeof(state->palette_rgb));
}

int stunrun_gsp_video_set_vram_words(stunrun_gsp_video_state_t *state,
                                     const uint16_t *vram_words,
                                     size_t vram_word_count)
{
    uint16_t *copy;

    if (state == NULL || vram_words == NULL ||
        !power_of_two(vram_word_count))
        return 0;
    copy = (uint16_t *)malloc(vram_word_count * sizeof(*copy));
    if (copy == NULL)
        return 0;
    memcpy(copy, vram_words, vram_word_count * sizeof(*copy));
    free(state->vram_words);
    state->vram_words = copy;
    state->vram_word_count = vram_word_count;
    return 1;
}

int stunrun_gsp_video_set_vram_bytes(stunrun_gsp_video_state_t *state,
                                     const uint8_t *vram_bytes,
                                     size_t vram_byte_count)
{
    uint16_t *copy;
    size_t word_count;
    size_t index;

    if (state == NULL || vram_bytes == NULL || vram_byte_count == 0u ||
        (vram_byte_count & 1u) != 0u)
        return 0;
    word_count = vram_byte_count / 2u;
    if (!power_of_two(word_count))
        return 0;
    copy = (uint16_t *)malloc(word_count * sizeof(*copy));
    if (copy == NULL)
        return 0;
    for (index = 0; index < word_count; index++)
        copy[index] = (uint16_t)vram_bytes[index * 2u] |
                      ((uint16_t)vram_bytes[index * 2u + 1u] << 8);
    free(state->vram_words);
    state->vram_words = copy;
    state->vram_word_count = word_count;
    return 1;
}

static int power_of_two(size_t value)
{
    return value != 0u && (value & (value - 1u)) == 0u;
}

void stunrun_gsp_video_init(stunrun_gsp_video_state_t *state)
{
    if (state == NULL)
        return;
    state->vram_words = NULL;
    state->vram_word_count = 0u;
    for (size_t i = 0; i < sizeof(state->palette_rgb); i++)
        state->palette_rgb[i] = 0u;
}

void stunrun_gsp_video_free(stunrun_gsp_video_state_t *state)
{
    if (state == NULL)
        return;
    free(state->vram_words);
    stunrun_gsp_video_init(state);
}

int stunrun_gsp_video_load(stunrun_gsp_video_state_t *state,
                           const char *vram_path, const char *palette_path)
{
    FILE *vram_file = NULL;
    FILE *palette_file = NULL;
    long vram_bytes;
    uint16_t *words = NULL;
    uint8_t palette_rgb[256u * 3u];
    size_t word_count;
    size_t i;
    if (state == NULL || vram_path == NULL || palette_path == NULL)
        return 0;
    vram_file = fopen(vram_path, "rb");
    palette_file = fopen(palette_path, "rb");
    if (vram_file == NULL || palette_file == NULL)
        goto fail;
    if (fseek(vram_file, 0, SEEK_END) != 0)
        goto fail;
    vram_bytes = ftell(vram_file);
    if (vram_bytes <= 0 || (vram_bytes % 2) != 0 ||
        fseek(vram_file, 0, SEEK_SET) != 0)
        goto fail;
    word_count = (size_t)vram_bytes / 2u;
    if (!power_of_two(word_count))
        goto fail;
    words = (uint16_t *)malloc(word_count * sizeof(*words));
    if (words == NULL || fread(palette_rgb, 1, sizeof(palette_rgb),
                               palette_file) != sizeof(palette_rgb) ||
        fgetc(palette_file) != EOF ||
        fread(words, sizeof(*words), word_count, vram_file) != word_count)
        goto fail_words;
    /* Exported words are little-endian, independent of host byte order. */
    for (i = 0; i < word_count; i++) {
        uint8_t *bytes = (uint8_t *)&words[i];
        words[i] = (uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8);
    }
    fclose(vram_file);
    fclose(palette_file);
    free(state->vram_words);
    state->vram_words = words;
    state->vram_word_count = word_count;
    memcpy(state->palette_rgb, palette_rgb, sizeof(state->palette_rgb));
    return 1;

fail_words:
    free(words);
fail:
    if (vram_file != NULL)
        fclose(vram_file);
    if (palette_file != NULL)
        fclose(palette_file);
    return 0;
}

int stunrun_gsp_video_render(const stunrun_gsp_video_state_t *state,
                             stunrun_renderer_t *renderer)
{
    if (state == NULL || renderer == NULL || state->vram_words == NULL ||
        state->vram_word_count == 0u)
        return 0;
    return stunrun_render_gsp_visible(renderer, state->vram_words,
                                      state->vram_word_count,
                                      state->palette_rgb);
}
