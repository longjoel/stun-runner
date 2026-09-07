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
    puts("native render boundary: all checks passed");
    return 0;
}
