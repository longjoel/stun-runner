#include "road_strip.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    stunrun_renderer_t renderer;
    stunrun_road_strip_t strip;
    stunrun_render_init(&renderer);
    stunrun_render_begin(&renderer, 1u, 0u, 0u, 0u);
    stunrun_road_strip_fixture(&strip);
    assert(stunrun_render_road_strip(&renderer, &strip));
    assert(renderer.pixels[((size_t)160u * STUNRUN_RENDER_WIDTH + 256u) * 3u]
           == strip.red);
    assert(renderer.pixels[0] == 0u);
    puts("road strip software backend: all checks passed");
    return 0;
}
