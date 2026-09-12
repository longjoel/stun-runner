#include "torus_lines.h"

#include <stddef.h>

#define TORUS_RED 177u
#define TORUS_GREEN 46u
#define TORUS_BLUE 36u

void stunrun_torus_lines_fixture(stunrun_torus_line_t *lines)
{
    if (lines == NULL)
        return;
    lines[0] = (stunrun_torus_line_t){55, 120, 201, 120};
    lines[1] = (stunrun_torus_line_t){312, 120, 467, 120};
    lines[2] = (stunrun_torus_line_t){161, 172, 282, 172};
}

int stunrun_render_torus_lines(stunrun_renderer_t *renderer,
                               const stunrun_torus_line_t *lines,
                               unsigned count)
{
    unsigned i;
    if (renderer == NULL || lines == NULL || count > STUNRUN_TORUS_LINE_COUNT)
        return 0;
    for (i = 0u; i < count; i++) {
        if (!stunrun_render_line(renderer, lines[i].x0, lines[i].y0,
                                 lines[i].x1, lines[i].y1, TORUS_RED,
                                 TORUS_GREEN, TORUS_BLUE))
            return 0;
    }
    return 1;
}

void stunrun_torus_rails_fixture(stunrun_torus_line_t *rails)
{
    if (rails == NULL)
        return;
    rails[0] = (stunrun_torus_line_t){11, 78, 180, 106};
    rails[1] = (stunrun_torus_line_t){501, 78, 332, 106};
    rails[2] = (stunrun_torus_line_t){181, 108, 201, 120};
    rails[3] = (stunrun_torus_line_t){331, 108, 312, 120};
}

int stunrun_render_torus_rails(stunrun_renderer_t *renderer,
                               const stunrun_torus_line_t *rails,
                               unsigned count)
{
    unsigned i;
    if (renderer == NULL || rails == NULL || count > STUNRUN_TORUS_RAIL_COUNT)
        return 0;
    for (i = 0u; i < count; i++) {
        if (!stunrun_render_line(renderer, rails[i].x0, rails[i].y0,
                                 rails[i].x1, rails[i].y1, TORUS_RED,
                                 TORUS_GREEN, TORUS_BLUE))
            return 0;
    }
    return 1;
}

void stunrun_torus_frags_fixture(stunrun_torus_line_t *frags)
{
    if (frags == NULL)
        return;
    frags[0] = (stunrun_torus_line_t){40, 122, 138, 154};
    frags[1] = (stunrun_torus_line_t){482, 122, 382, 154};
    frags[2] = (stunrun_torus_line_t){203, 121, 223, 127};
    frags[3] = (stunrun_torus_line_t){310, 121, 290, 127};
}

int stunrun_render_torus_frags(stunrun_renderer_t *renderer,
                               const stunrun_torus_line_t *frags,
                               unsigned count)
{
    unsigned i;
    if (renderer == NULL || frags == NULL || count > STUNRUN_TORUS_FRAG_COUNT)
        return 0;
    for (i = 0u; i < count; i++) {
        if (!stunrun_render_line(renderer, frags[i].x0, frags[i].y0,
                                 frags[i].x1, frags[i].y1, TORUS_RED,
                                 TORUS_GREEN, TORUS_BLUE))
            return 0;
    }
    return 1;
}
