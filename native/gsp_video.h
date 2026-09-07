#ifndef STUNRUN_NATIVE_GSP_VIDEO_H
#define STUNRUN_NATIVE_GSP_VIDEO_H

#include <stddef.h>
#include <stdint.h>

typedef struct stunrun_gsp_video_state {
    uint16_t *vram_words;
    size_t vram_word_count;
    uint8_t palette_rgb[256u * 3u];
} stunrun_gsp_video_state_t;

void stunrun_gsp_video_init(stunrun_gsp_video_state_t *state);
void stunrun_gsp_video_free(stunrun_gsp_video_state_t *state);
int stunrun_gsp_video_load(stunrun_gsp_video_state_t *state,
                           const char *vram_path, const char *palette_path);

#endif
