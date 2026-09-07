/* Literal GSP FIFO-to-work-buffer setup from the M5 consumer trace.
 *
 * This is intentionally a mechanism slice.  The FIFO payload's meaning and
 * the meaning of either destination buffer remain UNKNOWN.
 */
#ifndef STUNRUN_GSP_WORK_BUFFER_CONSUMER_H
#define STUNRUN_GSP_WORK_BUFFER_CONSUMER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STUNRUN_GSP_WORK_BUFFER_BASE 0xFFF6F650u
#define STUNRUN_GSP_WORK_BUFFER_TWIN 0xFFF70650u
#define STUNRUN_GSP_WORK_BUFFER_STRIDE 0x10u
#define STUNRUN_GSP_WORK_BUFFER_WRITE_BASE 0xFFF6F650u
#define STUNRUN_GSP_WORK_BUFFER_WRITE_TWIN 0xFFF70660u
#define STUNRUN_GSP_WORK_BUFFER_WRITE_STRIDE 0x20u
#define STUNRUN_GSP_WORK_BUFFER_WRITE_COUNT 128u
#define STUNRUN_GSP_DISPLAY_COPY_WORDS 256u
#define STUNRUN_GSP_DISPLAY_DEST_BASE 0xF5000000u
#define STUNRUN_GSP_DISPLAY_DEST_TWIN 0xF5800000u

typedef struct stunrun_gsp_work_buffer_write {
    uint32_t address;
    uint16_t value;
} stunrun_gsp_work_buffer_write_t;

/* The traced setup masks A4 to eight bits, scales it by 16, and adds the
 * result to both destination families. */
int stunrun_gsp_work_buffer_addresses(uint16_t a4, uint32_t *base_address,
                                      uint32_t *twin_address);

/* Address of one observed full-word write in a 128-write burst. */
int stunrun_gsp_work_buffer_write_addresses(size_t index,
                                            uint32_t *base_address,
                                            uint32_t *twin_address);

/* Pair independently supplied values with one observed 128-write burst.
 * Both event streams must have capacity for the whole requested burst; no
 * partial output is produced on invalid or undersized input. */
size_t stunrun_gsp_work_buffer_make_write_events(
    const uint16_t *base_values, const uint16_t *twin_values,
    size_t value_count, stunrun_gsp_work_buffer_write_t *base_events,
    size_t base_event_capacity, stunrun_gsp_work_buffer_write_t *twin_events,
    size_t twin_event_capacity);

/* Literal SLL 8h / OR byte-lane composition used by the loop. */
uint16_t stunrun_gsp_combine_byte_lanes(uint8_t high_lane,
                                        uint8_t low_lane);

/* Apply the traced adjacent-byte operation to separate caller-sized A0/A1
 * streams. The two inputs are intentionally independent: corresponding
 * writes were observed to differ, so this helper never duplicates one stream
 * into the other. */
size_t stunrun_gsp_expand_byte_lane_streams(
    const uint8_t *base_lanes, const uint8_t *twin_lanes, size_t lane_count,
    uint16_t *base_words, uint16_t *twin_words, size_t word_capacity);

/* Copy one independent 256-word stream through the observed sequential
 * display-memory transfer. Payload and destination device semantics remain
 * UNKNOWN. */
size_t stunrun_gsp_display_copy(const uint16_t *source, size_t source_words,
                                uint16_t *destination,
                                size_t destination_capacity);

#ifdef __cplusplus
}
#endif

#endif /* STUNRUN_GSP_WORK_BUFFER_CONSUMER_H */
