/* Native render slice for one tunnel-rib instance (entry-47 class).
 *
 * Provenance (see analysis/record-layout.md, analysis/track-hunt.md):
 * gated frame 1038 (tunnel-mouth entry, same-run screen) carries three chained
 * 62-byte records bound to course-table entry 47 (shape 0x2580, params
 * 0x9880/0x5DC00) at slots 0xFF9F2A/0xFFA386/0xFFA3C4, plus entry 151 at
 * 0xFFA402 and static entry 0 at 0xFF9C80. The lit rib faces below are measured
 * from that run's screenshot (/tmp/rec1038c/shot-1038.png): each side band is
 * two flat-shaded quads (facets) meeting at a kink near y=123. Corners are
 * screen-space pixels; color is the sampled band median (151,43,34).
 *
 * This renders the rib faces as ordinary road strips through the shared
 * software backend. It does not claim the GSP span encoding.
 */
#ifndef STUNRUN_NATIVE_TUNNEL_RIB_H
#define STUNRUN_NATIVE_TUNNEL_RIB_H

#include "road_strip.h"

/* Facet quads, near (lower) pair at the array head (nearest: the frame
 * renderer paints strips back-to-front from the tail). The pairs share
 * only the y123 edge. */
#define STUNRUN_RIB_FACET_COUNT 4u

void stunrun_tunnel_rib_frame(stunrun_road_frame_t *frame);

int stunrun_render_tunnel_rib(stunrun_renderer_t *renderer,
                              const stunrun_road_frame_t *frame);

#endif
