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

#define STUNRUN_ROAD_FIFO_WORDS 384u
#define STUNRUN_ROAD_FIFO_PC 0x02248Eu
#define STUNRUN_ROAD_FIFO_DEST 0xC0000Cu

/* Copy one observed complete base-buffer burst into a host FIFO fixture.
 * Returns STUNRUN_ROAD_FIFO_WORDS only for valid complete buffers. */
size_t stunrun_road_fifo_drain(const uint16_t *base, size_t words,
                               uint16_t *fifo_out, size_t fifo_cap);

#ifdef __cplusplus
}
#endif

#endif /* STUNRUN_ROAD_FIFO_H */
