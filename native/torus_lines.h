/* Native render slice for torus wireframe floor lines (entry-41 class).
 *
 * Provenance (see analysis/record-layout.md): gated frame 1920 (wireframe
 * torus, same-run screen, score 1080) carries twelve chained 62-byte
 * records bound to course-table entries 44/45 (run 41-82, shape 0x2580;
 * see the entry-41 ASSUMPTION CONFLICT in analysis/record-layout.md)
 * with heap status flags 0x1800/0x1A00 (vs 0x0/0x200 for solid mouth
 * ribs at frame 1038):
 * same template class, wireframe render mode. The three floor cross-lines
 * below are measured from that run's screenshot
 * (/tmp/rec1920b/shot-1920.png): left/right spans at y=120 with a center
 * opening (x 201-312), and one center span at y=172. Sampled line color
 * (177,46,36). Exact record-to-line mapping is open; the diffuse radial
 * fragments elsewhere in the frame are not modeled here.
 *
 * Rendered as Bresenham lines through the shared software backend.
 *
 * The side rails below share the same color and backend. Provenance: the
 * same shot-1920 carries two steep diagonal rails (left x11/y78 to
 * x180/y106, right mirrored about x256) plus short connectors from the
 * rail ends to the floor-line corners (181,108)-(201,120) and
 * (331,108)-(312,120), all in (177,46,36). The teal arc mass is not
 * modeled here.
 *
 * The below-floor fragments below share the same color and backend.
 * Provenance: the same shot-1920 carries four short fragments just under
 * the floor line — outer left (40,122)-(138,154), outer right
 * (482,122)-(382,154), inner left (203,121)-(223,127), inner right
 * (310,121)-(290,127) — the rails continuing past the floor line with a
 * perspective kink (slope ~6 above, ~3 below).
 */
#ifndef STUNRUN_NATIVE_TORUS_LINES_H
#define STUNRUN_NATIVE_TORUS_LINES_H

#include "render.h"

#define STUNRUN_TORUS_LINE_COUNT 3u
#define STUNRUN_TORUS_RAIL_COUNT 4u
#define STUNRUN_TORUS_FRAG_COUNT 4u

typedef struct stunrun_torus_line {
    int x0, y0, x1, y1;
} stunrun_torus_line_t;

void stunrun_torus_lines_fixture(stunrun_torus_line_t *lines);
void stunrun_torus_rails_fixture(stunrun_torus_line_t *rails);
void stunrun_torus_frags_fixture(stunrun_torus_line_t *frags);

int stunrun_render_torus_lines(stunrun_renderer_t *renderer,
                               const stunrun_torus_line_t *lines,
                               unsigned count);
int stunrun_render_torus_rails(stunrun_renderer_t *renderer,
                               const stunrun_torus_line_t *rails,
                               unsigned count);
int stunrun_render_torus_frags(stunrun_renderer_t *renderer,
                               const stunrun_torus_line_t *frags,
                               unsigned count);

#endif
