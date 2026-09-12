#include "craft.h"

#include <assert.h>
#include <stdio.h>

/* Measured masks from the gated frame-1038 screenshot
 * (/tmp/rec1038c/shot-1038.png). CANOPY: x 236-266, y 146-160 (white).
 * FUSE: x 225-280, y 158-182 (craft red). Fuselage rows 9-13 (y167-171)
 * carry dark panel stripes and are excluded from scoring. */
static const char *CRAFT_CANOPY[] = {
    "0000000000000001000000000000000",
    "0000000000001111111000000000000",
    "0000000001111111111110000000000",
    "0000001111111111111111110000000",
    "0000011111111110111111111000000",
    "0000111111110000000111111100000",
    "0000111110000000000001111110000",
    "0001111100000000000000111111000",
    "0011111000000000000000011111100",
    "0001110000000000000000001111000",
    "0001111000000000000000011110000",
    "0000111100000000000000111100000",
    "0000011100000000000000011100000",
    "0000000000000000000000000000000",
    "0000000000000000000000000000000",
};
static const char *CRAFT_FUSE[] = {
    "00000000000000000001111100000000000000000000000000000000",
    "00000000000000001111111100000000001100000000000000000000",
    "00000000000000111111111000000000000111000000000000000000",
    "00000000000111111111110000000001111111111000000000000000",
    "00000000011111111111111111111111111111111110000000000000",
    "00000001111111111111111111111111111111111111110000000000",
    "00000111111111111111111111111111111111111111111100000000",
    "00001111111111111111111111111111111111111111111100000000",
    "00011111111111111111111111111111111111111111111110000000",
    "00111110000000000000000000000000000000000000011111000000",
    "01111100000000000000000000000000000000000000000111100000",
    "11110000000000000000000000000000000000000000000011111000",
    "11100000000000000000000000000000000000000000000000111100",
    "11001111111111111111111111111111111111111111111100011110",
    "10011111111111111111111111111111111111111111111111001111",
    "01111111111111111111111111111111111111111111111111110011",
    "11111111111111111111111111111111111111111111111111111101",
    "11111111111111111111111111111111111111111111111111111111",
    "11111111111111111111111111111111111111111111111111111111",
    "11111111111111111111111111111111111111111111111111111111",
    "11111111111111111111111111111111111111111111111111111111",
    "11111111111111111111111111111111111111111111111111111111",
    "11111111111111111111111111111111111111111111111111111111",
    "11111111111111111111111111111111111111111111111111111111",
    "11111111111111111111111111111111111111111111111111111111",
};

static int pixel_is(const stunrun_renderer_t *renderer, unsigned x,
                    unsigned y, unsigned red, unsigned green, unsigned blue)
{
    size_t at = ((size_t)y * STUNRUN_RENDER_WIDTH + x) * 3u;
    return renderer->pixels[at] == red && renderer->pixels[at + 1u] == green &&
           renderer->pixels[at + 2u] == blue;
}

static int is_white(const stunrun_renderer_t *renderer, unsigned x,
                    unsigned y)
{
    return pixel_is(renderer, x, y, 159u, 153u, 154u);
}

static int is_red(const stunrun_renderer_t *renderer, unsigned x, unsigned y)
{
    return pixel_is(renderer, x, y, 191u, 0u, 0u);
}

static int is_wing(const stunrun_renderer_t *renderer, unsigned x,
                   unsigned y)
{
    return pixel_is(renderer, x, y, 207u, 0u, 0u);
}

int main(void)
{
    stunrun_renderer_t renderer;
    stunrun_road_frame_t frame;
    unsigned x, y, hit, gold, both;
    stunrun_render_init(&renderer);
    stunrun_render_begin(&renderer, 1u, 0u, 0u, 0u);
    stunrun_craft_frame(&frame);
    assert(frame.count == STUNRUN_CRAFT_PART_COUNT);
    assert(stunrun_render_craft(&renderer, &frame));
    /* Part probes: canopy lobes white, nose/fuselage red, gaps clear. */
    assert(is_white(&renderer, 241u, 152u));
    assert(is_white(&renderer, 261u, 152u));
    assert(is_white(&renderer, 250u, 148u));
    assert(is_red(&renderer, 252u, 157u));
    assert(is_red(&renderer, 250u, 165u));
    assert(!is_white(&renderer, 250u, 157u));
    assert(!is_red(&renderer, 230u, 150u));
    /* Wing stubs: mid-span both sides, tips near the measured extrema,
     * clear air just beyond the tips. */
    assert(is_wing(&renderer, 236u, 164u));
    assert(is_wing(&renderer, 260u, 164u));
    assert(is_wing(&renderer, 230u, 168u));
    assert(is_wing(&renderer, 233u, 166u));
    assert(is_wing(&renderer, 262u, 166u));
    assert(!is_wing(&renderer, 220u, 164u));
    assert(!is_wing(&renderer, 276u, 164u));
    assert(!is_wing(&renderer, 248u, 164u));
    /* Canopy agreement over its 15 rows. */
    hit = 0u;
    gold = 0u;
    both = 0u;
    for (y = 0u; y < 15u; y++) {
        for (x = 0u; x < 31u; x++) {
            int want = CRAFT_CANOPY[y][x] == '1';
            int got = is_white(&renderer, 236u + x, 146u + y);
            hit += (unsigned)got;
            gold += (unsigned)want;
            both += (unsigned)(got && want);
        }
    }
    assert(gold > 0u && hit > 0u);
    assert(both * 100u >= gold * 70u);
    assert(both * 100u >= hit * 70u);
    /* Fuselage agreement, skipping stripe rows y167-171. */
    hit = 0u;
    gold = 0u;
    both = 0u;
    for (y = 0u; y < 25u; y++) {
        unsigned yy = 158u + y;
        if (yy >= 167u && yy <= 171u)
            continue;
        for (x = 0u; x < 56u; x++) {
            int want = CRAFT_FUSE[y][x] == '1';
            int got = is_red(&renderer, 225u + x, yy);
            hit += (unsigned)got;
            gold += (unsigned)want;
            both += (unsigned)(got && want);
        }
    }
    assert(gold > 0u && hit > 0u);
    assert(both * 100u >= gold * 75u);
    assert(both * 100u >= hit * 75u);
    /* Wing population: both stubs render a solid tapered mass. */
    hit = 0u;
    for (y = 159u; y <= 169u; y++) {
        for (x = 220u; x <= 276u; x++)
            hit += (unsigned)is_wing(&renderer, x, y);
    }
    assert(hit >= 150u);
    /* Panel-stripe band: black core, red preserved at the edges. */
    assert(pixel_is(&renderer, 250u, 168u, 0u, 0u, 0u));
    assert(pixel_is(&renderer, 240u, 169u, 0u, 0u, 0u));
    assert(pixel_is(&renderer, 260u, 169u, 0u, 0u, 0u));
    assert(!pixel_is(&renderer, 230u, 168u, 0u, 0u, 0u));
    assert(is_red(&renderer, 250u, 166u));
    puts("craft software backend: all checks passed");
    return 0;
}
