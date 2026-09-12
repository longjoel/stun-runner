#include "scene.h"

#include <stddef.h>

#include "craft.h"
#include "tunnel_rib.h"
#include "vista_quad.h"

int stunrun_render_mouth_entry(stunrun_renderer_t *renderer)
{
    stunrun_road_frame_t frame;

    if (renderer == NULL)
        return 0;
    stunrun_render_begin(renderer, 1u, 0u, 0u, 0u);
    /* Far to near: vista, rib, craft. Each slice paints its own strips
     * in its own back-to-front order. */
    stunrun_vista_quad_frame(&frame);
    if (!stunrun_render_vista_quad(renderer, &frame))
        return 0;
    stunrun_tunnel_rib_frame(&frame);
    if (!stunrun_render_tunnel_rib(renderer, &frame))
        return 0;
    stunrun_craft_frame(&frame);
    if (!stunrun_render_craft(renderer, &frame))
        return 0;
    return 1;
}
