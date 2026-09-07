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
    puts("native gsp video boundary: all checks passed");
    return 0;
}
