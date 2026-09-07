/* Settled road-buffer to GSP-FIFO drain, in C.
 *
 * Literal mechanism only. The FIFO payload's field semantics and the GSP's
 * interpretation remain unresolved.
 */
#ifndef STUNRUN_ROAD_FIFO_H
#define STUNRUN_ROAD_FIFO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STUNRUN_ROAD_FIFO_SOURCE_WORDS 384u
#define STUNRUN_ROAD_FIFO_WRITES 192u
#define STUNRUN_ROAD_FIFO_PC 0x02248Eu
#define STUNRUN_ROAD_FIFO_DEST 0xC0000Cu

/* Emit the observed tap-level FIFO shape for one complete base-buffer burst:
 * 384 source halfword reads produce 192 writes matching source[0], source[2],
 * ... source[382]. This is a bus/lane contract, not payload semantics. */
size_t stunrun_road_fifo_drain(const uint16_t *base, size_t words,
                               uint16_t *fifo_out, size_t fifo_cap);

/* Drain a bounded source sub-range using the same observed even-word lane.
 * This represents a burst observed across multiple frames: source_offset
 * must be even, and the returned words are base[source_offset],
 * base[source_offset + 2], ... . */
size_t stunrun_road_fifo_drain_range(const uint16_t *base, size_t words,
                                     size_t source_offset,
                                     size_t source_count,
                                     uint16_t *fifo_out, size_t fifo_cap);

#ifdef __cplusplus
}
#endif

#endif /* STUNRUN_ROAD_FIFO_H */
