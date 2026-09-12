/* Native render slice for the tunnel-mouth entry combo (frame 1038).
 *
 * Provenance (see analysis/record-layout.md): the gated frame-1038
 * screenshot (tunnel-mouth entry, same-run screen) carries the far vista
 * quad, the tunnel-rib band, and the level-flight craft core together.
 * Each layer is owned by its own slice (vista_quad, tunnel_rib, craft);
 * this module only fixes their shared paint order, far to near:
 * vista first, rib second, craft last. The torus wireframe belongs to a
 * different section (frame 1920) and is not composited here.
 */
#ifndef STUNRUN_NATIVE_SCENE_H
#define STUNRUN_NATIVE_SCENE_H

#include "render.h"

/* Render the mouth-entry combo onto renderer in far-to-near order.
 * Returns nonzero on success. */
int stunrun_render_mouth_entry(stunrun_renderer_t *renderer);

#endif
