#include "gsp_video.h"

#include <stdio.h>
#include <stdlib.h>

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
    FILE *vram_file;
    FILE *palette_file;
    long vram_bytes;
    uint16_t *words;
    size_t word_count;
    size_t i;
    if (state == NULL || vram_path == NULL || palette_path == NULL)
        return 0;
    stunrun_gsp_video_free(state);
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
    if (words == NULL || fread(state->palette_rgb, 1,
                               sizeof(state->palette_rgb), palette_file) !=
                            sizeof(state->palette_rgb) ||
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
    state->vram_words = words;
    state->vram_word_count = word_count;
    return 1;

fail_words:
    free(words);
fail:
    if (vram_file != NULL)
        fclose(vram_file);
    if (palette_file != NULL)
        fclose(palette_file);
    stunrun_gsp_video_init(state);
    return 0;
}
