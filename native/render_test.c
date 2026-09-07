#include "render.h"

#include <stdio.h>

int main(void)
{
    stunrun_renderer_t renderer;
    stunrun_render_init(&renderer);
    stunrun_render_begin(&renderer, 17u, 0u, 0u, 0u);
    if (renderer.frame != 17u || stunrun_render_hash(&renderer) !=
        0x85476dc5u)
        return 1;
    if (!stunrun_render_set_pixel(&renderer, 319u, 239u, 1u, 2u, 3u) ||
        stunrun_render_set_pixel(&renderer, 320u, 0u, 1u, 2u, 3u) ||
        stunrun_render_set_pixel(&renderer, 0u, 240u, 1u, 2u, 3u))
        return 1;
    if (stunrun_render_hash(&renderer) == 0x85476dc5u)
        return 1;
    puts("native render boundary: all checks passed");
    return 0;
}
