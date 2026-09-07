/* Byte-wise road-buffer arithmetic observed in the 68010 listing. */
#ifndef STUNRUN_ROAD_BUFFER_MATH_H
#define STUNRUN_ROAD_BUFFER_MATH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Provenance: maincpu listing at 0x0298C0 and 0x0298FA.
 * Each routine walks 0x300 bytes, reading the twin byte and updating the
 * base byte. C uint8_t assignment gives the observed 68010 byte wraparound.
 * The caller supplies the active byte count so the slice is independently
 * testable; the observed production count is STUNRUN_ROAD_BUFFER_BYTES.
 */
#define STUNRUN_ROAD_BUFFER_BYTES 0x300u

void stunrun_road_buffer_subtract(uint8_t *base, const uint8_t *twin,
                                  size_t bytes);
void stunrun_road_buffer_add(uint8_t *base, const uint8_t *twin,
                             size_t bytes);

#ifdef __cplusplus
}
#endif

#endif /* STUNRUN_ROAD_BUFFER_MATH_H */
