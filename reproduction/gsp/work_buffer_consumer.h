/* Literal GSP FIFO-to-work-buffer setup from the M5 consumer trace.
 *
 * This is intentionally a mechanism slice.  The FIFO payload's meaning and
 * the meaning of either destination buffer remain UNKNOWN.
 */
#ifndef STUNRUN_GSP_WORK_BUFFER_CONSUMER_H
#define STUNRUN_GSP_WORK_BUFFER_CONSUMER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STUNRUN_GSP_WORK_BUFFER_BASE 0xFFF6F650u
#define STUNRUN_GSP_WORK_BUFFER_TWIN 0xFFF70650u
#define STUNRUN_GSP_WORK_BUFFER_STRIDE 0x10u

/* The traced setup masks A4 to eight bits, scales it by 16, and adds the
 * result to both destination families. */
int stunrun_gsp_work_buffer_addresses(uint16_t a4, uint32_t *base_address,
                                      uint32_t *twin_address);

/* Literal SLL 8h / OR byte-lane composition used by the loop. */
uint16_t stunrun_gsp_combine_byte_lanes(uint8_t high_lane,
                                        uint8_t low_lane);

#ifdef __cplusplus
}
#endif

#endif /* STUNRUN_GSP_WORK_BUFFER_CONSUMER_H */
