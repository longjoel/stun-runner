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

int stunrun_road_buffer_copy_scaled(const uint8_t *source, uint8_t *base,
                                    uint8_t *twin, uint8_t divisor,
                                    size_t bytes)
{
    size_t i;

    if (source == NULL || base == NULL || twin == NULL || divisor == 0u) {
        return 0;
    }
    for (i = 0; i < bytes; i++) {
        base[i] = source[i];
        twin[i] = (uint8_t)(source[i] / divisor);
    }
    return 1;
}
