/* See road_fifo.h for provenance and confidence notes. */
#include "road_fifo.h"

size_t stunrun_road_fifo_drain(const uint16_t *base, size_t words,
                               uint16_t *fifo_out, size_t fifo_cap)
{
    size_t i;

    if (base == NULL || fifo_out == NULL ||
        words != STUNRUN_ROAD_FIFO_SOURCE_WORDS ||
        fifo_cap < STUNRUN_ROAD_FIFO_WRITES) {
        return 0;
    }
    for (i = 0; i < STUNRUN_ROAD_FIFO_WRITES; i++) {
        fifo_out[i] = base[i * 2u];
    }
    return STUNRUN_ROAD_FIFO_WRITES;
}

size_t stunrun_road_fifo_drain_range(const uint16_t *base, size_t words,
                                     size_t source_offset,
                                     size_t source_count,
                                     uint16_t *fifo_out, size_t fifo_cap)
{
    size_t writes;
    size_t i;

    if (base == NULL || fifo_out == NULL ||
        source_offset > words || source_count > words - source_offset ||
        (source_offset & 1u) != 0u || (source_count & 1u) != 0u)
        return 0u;
    writes = source_count / 2u;
    if (fifo_cap < writes)
        return 0u;
    for (i = 0; i < writes; i++)
        fifo_out[i] = base[source_offset + i * 2u];
    return writes;
}
