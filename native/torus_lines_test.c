#include "torus_lines.h"

#include <assert.h>
#include <stdio.h>

static int rendered_wire(const stunrun_renderer_t *renderer, unsigned x,
                         unsigned y)
{
    size_t at = ((size_t)y * STUNRUN_RENDER_WIDTH + x) * 3u;
    return renderer->pixels[at] == 177u && renderer->pixels[at + 1u] == 46u &&
           renderer->pixels[at + 2u] == 36u;
}

int main(int argc, char **argv)
{
    stunrun_renderer_t renderer;
    stunrun_torus_line_t lines[STUNRUN_TORUS_LINE_COUNT];
    unsigned hit;
    stunrun_render_init(&renderer);
    stunrun_render_begin(&renderer, 1u, 0u, 0u, 0u);
    stunrun_torus_lines_fixture(lines);
    assert(lines[0].x0 == 55 && lines[0].x1 == 201 && lines[0].y0 == 120);
    assert(lines[1].x0 == 312 && lines[1].x1 == 467 && lines[1].y0 == 120);
    assert(lines[2].x0 == 161 && lines[2].x1 == 282 && lines[2].y0 == 172);
    assert(stunrun_render_torus_lines(&renderer, lines,
                                      STUNRUN_TORUS_LINE_COUNT));
    /* Endpoints and midpoints of all three measured spans are drawn. */
    assert(rendered_wire(&renderer, 55u, 120u));
    assert(rendered_wire(&renderer, 201u, 120u));
    assert(rendered_wire(&renderer, 128u, 120u));
    assert(rendered_wire(&renderer, 312u, 120u));
    assert(rendered_wire(&renderer, 467u, 120u));
    assert(rendered_wire(&renderer, 390u, 120u));
    assert(rendered_wire(&renderer, 161u, 172u));
    assert(rendered_wire(&renderer, 282u, 172u));
    assert(rendered_wire(&renderer, 220u, 172u));
    /* The center opening and the field above/below stay clear. */
    assert(!rendered_wire(&renderer, 256u, 120u));
    assert(!rendered_wire(&renderer, 256u, 100u));
    assert(!rendered_wire(&renderer, 256u, 150u));
    assert(!rendered_wire(&renderer, 100u, 172u));
    /* Pixel count matches three 1px spans within raster tolerance. */
    hit = 0u;
    for (unsigned y = 0u; y < STUNRUN_RENDER_HEIGHT; y++)
        for (unsigned x = 0u; x < STUNRUN_RENDER_WIDTH; x++)
            hit += (unsigned)rendered_wire(&renderer, x, y);
    assert(hit >= 420u && hit <= 430u);
    /* Side rails on a fresh frame: endpoints, midpoints, clear center. */
    {
        stunrun_renderer_t rail_renderer;
        stunrun_torus_line_t rails[STUNRUN_TORUS_RAIL_COUNT];
        stunrun_render_init(&rail_renderer);
        stunrun_render_begin(&rail_renderer, 1u, 0u, 0u, 0u);
        stunrun_torus_rails_fixture(rails);
        assert(rails[0].x0 == 11 && rails[0].y0 == 78);
        assert(rails[1].x0 == 501 && rails[1].y0 == 78);
        assert(stunrun_render_torus_rails(&rail_renderer, rails,
                                          STUNRUN_TORUS_RAIL_COUNT));
        assert(rendered_wire(&rail_renderer, 11u, 78u));
        assert(rendered_wire(&rail_renderer, 180u, 106u));
        assert(rendered_wire(&rail_renderer, 95u, 92u));
        assert(rendered_wire(&rail_renderer, 501u, 78u));
        assert(rendered_wire(&rail_renderer, 332u, 106u));
        assert(rendered_wire(&rail_renderer, 417u, 92u));
        assert(rendered_wire(&rail_renderer, 181u, 108u));
        assert(rendered_wire(&rail_renderer, 201u, 120u));
        assert(rendered_wire(&rail_renderer, 331u, 108u));
        assert(rendered_wire(&rail_renderer, 312u, 120u));
        assert(!rendered_wire(&rail_renderer, 256u, 92u));
        assert(!rendered_wire(&rail_renderer, 256u, 60u));
        hit = 0u;
        for (unsigned y = 0u; y < STUNRUN_RENDER_HEIGHT; y++)
            for (unsigned x = 0u; x < STUNRUN_RENDER_WIDTH; x++)
                hit += (unsigned)rendered_wire(&rail_renderer, x, y);
        assert(hit >= 375u && hit <= 392u);
    }
    /* Below-floor fragments on a fresh frame: endpoints and count. */
    {
        stunrun_renderer_t frag_renderer;
        stunrun_torus_line_t frags[STUNRUN_TORUS_FRAG_COUNT];
        stunrun_render_init(&frag_renderer);
        stunrun_render_begin(&frag_renderer, 1u, 0u, 0u, 0u);
        stunrun_torus_frags_fixture(frags);
        assert(frags[0].x0 == 40 && frags[0].y0 == 122);
        assert(frags[1].x0 == 482 && frags[1].y0 == 122);
        assert(stunrun_render_torus_frags(&frag_renderer, frags,
                                          STUNRUN_TORUS_FRAG_COUNT));
        assert(rendered_wire(&frag_renderer, 40u, 122u));
        assert(rendered_wire(&frag_renderer, 138u, 154u));
        assert(rendered_wire(&frag_renderer, 89u, 138u));
        assert(rendered_wire(&frag_renderer, 482u, 122u));
        assert(rendered_wire(&frag_renderer, 382u, 154u));
        assert(rendered_wire(&frag_renderer, 432u, 138u));
        assert(rendered_wire(&frag_renderer, 203u, 121u));
        assert(rendered_wire(&frag_renderer, 223u, 127u));
        assert(rendered_wire(&frag_renderer, 310u, 121u));
        assert(rendered_wire(&frag_renderer, 290u, 127u));
        assert(!rendered_wire(&frag_renderer, 256u, 138u));
        assert(!rendered_wire(&frag_renderer, 256u, 110u));
        hit = 0u;
        for (unsigned y = 0u; y < STUNRUN_RENDER_HEIGHT; y++)
            for (unsigned x = 0u; x < STUNRUN_RENDER_WIDTH; x++)
                hit += (unsigned)rendered_wire(&frag_renderer, x, y);
        assert(hit >= 232u && hit <= 250u);
    }
    if (argc > 2) {
        /* Coarse ASCII preview (64x30): 'L' wire, '.' black. Renders the
         * floor lines plus the rails together for eye-verification. */
        stunrun_renderer_t combo;
        stunrun_torus_line_t rails[STUNRUN_TORUS_RAIL_COUNT];
        stunrun_torus_line_t frags[STUNRUN_TORUS_FRAG_COUNT];
        unsigned gx, gy;
        stunrun_render_init(&combo);
        stunrun_render_begin(&combo, 1u, 0u, 0u, 0u);
        assert(stunrun_render_torus_lines(&combo, lines,
                                          STUNRUN_TORUS_LINE_COUNT));
        stunrun_torus_rails_fixture(rails);
        assert(stunrun_render_torus_rails(&combo, rails,
                                          STUNRUN_TORUS_RAIL_COUNT));
        stunrun_torus_frags_fixture(frags);
        assert(stunrun_render_torus_frags(&combo, frags,
                                          STUNRUN_TORUS_FRAG_COUNT));
        if (argc > 1)
            assert(stunrun_render_write_ppm(&combo, argv[1]));
        for (gy = 0u; gy < 30u; gy++) {
            char line[65];
            for (gx = 0u; gx < 64u; gx++) {
                size_t at = ((size_t)(gy * 8u + 4u) * STUNRUN_RENDER_WIDTH +
                             gx * 8u + 4u) * 3u;
                line[gx] = (combo.pixels[at] == 177u &&
                            combo.pixels[at + 1u] == 46u &&
                            combo.pixels[at + 2u] == 36u) ? 'L' : '.';
            }
            line[64] = '\0';
            puts(line);
        }
    }
    puts("torus lines software backend: all checks passed");
    return 0;
}
