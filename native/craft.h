/* Native render slice for the player craft core, level flight (unproven record).
 *
 * Provenance: gated frame 1038 (mouth entry, same-run screen) shows the craft
 * solid and level at screen center-bottom. No 62B course record is proven to
 * drive it (entry-146 rebinds often but is never resident with craft-like
 * constancy; the craft may come from object ROM instead), so this fixture is
 * measured screen geometry only: canopy top + two lobes (off-white
 * 159,153,154), nose + fuselage core (craft red 191,0,0), wing stubs
 * (modal shade 207,0,0), panel-stripe band (black trapezoid y167-170).
 * Wing gradient/grey-speck detail, stripe edge remnants (2px), the red
 * right flank (x271-275, y167-168), and outlines are open.
 *
 * Rendered as road strips through the shared software backend.
 */
#ifndef STUNRUN_NATIVE_CRAFT_H
#define STUNRUN_NATIVE_CRAFT_H

#include "road_strip.h"

#define STUNRUN_CRAFT_PART_COUNT 9u

void stunrun_craft_frame(stunrun_road_frame_t *frame);

int stunrun_render_craft(stunrun_renderer_t *renderer,
                         const stunrun_road_frame_t *frame);

#endif
