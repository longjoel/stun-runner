/* See road_buffer_math.h for the observed listing contract. */
#include "road_buffer_math.h"

void stunrun_road_buffer_subtract(uint8_t *base, const uint8_t *twin,
                                  size_t bytes)
{
    size_t i;

    if (base == NULL || twin == NULL) {
        return;
    }
    for (i = 0; i < bytes; i++) {
        base[i] = (uint8_t)(base[i] - twin[i]);
    }
}

void stunrun_road_buffer_add(uint8_t *base, const uint8_t *twin,
                             size_t bytes)
{
    size_t i;

    if (base == NULL || twin == NULL) {
        return;
    }
    for (i = 0; i < bytes; i++) {
        base[i] = (uint8_t)(base[i] + twin[i]);
    }
}
