#include "scene.h"

#include <assert.h>
#include <stdio.h>

static int pixel_is(const stunrun_renderer_t *renderer, unsigned x,
                    unsigned y, unsigned red, unsigned green, unsigned blue)
{
    size_t at = ((size_t)y * STUNRUN_RENDER_WIDTH + x) * 3u;
    return renderer->pixels[at] == red && renderer->pixels[at + 1u] == green &&
           renderer->pixels[at + 2u] == blue;
}

int main(int argc, char **argv)
{
    stunrun_renderer_t renderer;
    stunrun_render_init(&renderer);
    assert(stunrun_render_mouth_entry(&renderer));
    /* Vista far layer survives where nothing covers it. */
    assert(pixel_is(&renderer, 251u, 100u, 182u, 151u, 120u));
    /* Rib band survives between vista top and craft nose. */
    assert(pixel_is(&renderer, 225u, 128u, 151u, 43u, 34u));
    assert(pixel_is(&renderer, 275u, 128u, 151u, 43u, 34u));
    /* Craft near layer paints last: canopy white, fuselage red, wings. */
    assert(pixel_is(&renderer, 250u, 148u, 159u, 153u, 154u));
    assert(pixel_is(&renderer, 250u, 165u, 191u, 0u, 0u));
    assert(pixel_is(&renderer, 236u, 164u, 207u, 0u, 0u));
    assert(pixel_is(&renderer, 260u, 164u, 207u, 0u, 0u));
    /* Clear zones stay clear: frame corner and rib center gap. */
    assert(pixel_is(&renderer, 10u, 10u, 0u, 0u, 0u));
    assert(!pixel_is(&renderer, 248u, 124u, 151u, 43u, 34u));
    if (argc > 1)
        assert(stunrun_render_write_ppm(&renderer, argv[1]));
    if (argc > 2) {
        /* Coarse ASCII preview (64x30) for stdout eye-verification:
         * '.' black, 'V' vista tan, 'R' rib red, 'W' craft white,
         * 'C' craft red, 'r' wing red, '#' anything else. */
        unsigned gx, gy;
        for (gy = 0u; gy < 30u; gy++) {
            char line[65];
            for (gx = 0u; gx < 64u; gx++) {
                unsigned x = gx * 8u + 4u, y = gy * 8u + 4u;
                char c = '.';
                if (pixel_is(&renderer, x, y, 182u, 151u, 120u))
                    c = 'V';
                else if (pixel_is(&renderer, x, y, 151u, 43u, 34u))
                    c = 'R';
                else if (pixel_is(&renderer, x, y, 159u, 153u, 154u))
                    c = 'W';
                else if (pixel_is(&renderer, x, y, 191u, 0u, 0u))
                    c = 'C';
                else if (pixel_is(&renderer, x, y, 207u, 0u, 0u))
                    c = 'r';
                else if (!pixel_is(&renderer, x, y, 0u, 0u, 0u))
                    c = '#';
                line[gx] = c;
            }
            line[64] = '\0';
            puts(line);
        }
    }
    puts("mouth-entry scene software backend: all checks passed");
    return 0;
}
