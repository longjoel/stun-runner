/* See road_fifo.h for provenance and confidence notes. */
#include "road_fifo.h"

size_t stunrun_road_fifo_drain(const uint16_t *base, size_t words,
                               uint16_t *fifo_out, size_t fifo_cap)
{
    size_t i;

    if (base == NULL || fifo_out == NULL || words != STUNRUN_ROAD_FIFO_WORDS ||
        fifo_cap < STUNRUN_ROAD_FIFO_WORDS) {
        return 0;
    }
    for (i = 0; i < STUNRUN_ROAD_FIFO_WORDS; i++) {
        fifo_out[i] = base[i];
    }
    return STUNRUN_ROAD_FIFO_WORDS;
}
