/* See work_buffer_consumer.h for provenance and scope. */
#include "work_buffer_consumer.h"

#include <limits.h>
#include <stddef.h>

int stunrun_gsp_work_buffer_addresses(uint16_t a4, uint32_t *base_address,
                                      uint32_t *twin_address)
{
    uint32_t offset = (uint32_t)(a4 & 0x00FFu) *
                      STUNRUN_GSP_WORK_BUFFER_STRIDE;

    if (base_address == NULL || twin_address == NULL ||
        UINT32_MAX - STUNRUN_GSP_WORK_BUFFER_BASE < offset ||
        UINT32_MAX - STUNRUN_GSP_WORK_BUFFER_TWIN < offset)
        return 0;
    *base_address = STUNRUN_GSP_WORK_BUFFER_BASE + offset;
    *twin_address = STUNRUN_GSP_WORK_BUFFER_TWIN + offset;
    return 1;
}

int stunrun_gsp_work_buffer_write_addresses(size_t index,
                                            uint32_t *base_address,
                                            uint32_t *twin_address)
{
    uint32_t offset;

    if (base_address == NULL || twin_address == NULL ||
        index >= STUNRUN_GSP_WORK_BUFFER_WRITE_COUNT)
        return 0;
    offset = (uint32_t)index * STUNRUN_GSP_WORK_BUFFER_WRITE_STRIDE;
    *base_address = STUNRUN_GSP_WORK_BUFFER_WRITE_BASE + offset;
    *twin_address = STUNRUN_GSP_WORK_BUFFER_WRITE_TWIN + offset;
    return 1;
}

size_t stunrun_gsp_work_buffer_make_write_events(
    const uint16_t *base_values, const uint16_t *twin_values,
    size_t value_count, stunrun_gsp_work_buffer_write_t *base_events,
    size_t base_event_capacity, stunrun_gsp_work_buffer_write_t *twin_events,
    size_t twin_event_capacity)
{
    size_t i;

    if (base_values == NULL || twin_values == NULL || value_count == 0u ||
        value_count > STUNRUN_GSP_WORK_BUFFER_WRITE_COUNT ||
        base_events == NULL || twin_events == NULL ||
        base_event_capacity < value_count || twin_event_capacity < value_count)
        return 0u;
    for (i = 0; i < value_count; i++) {
        uint32_t base_address;
        uint32_t twin_address;
        if (!stunrun_gsp_work_buffer_write_addresses(
                i, &base_address, &twin_address))
            return 0u;
        base_events[i].address = base_address;
        base_events[i].value = base_values[i];
        twin_events[i].address = twin_address;
        twin_events[i].value = twin_values[i];
    }
    return value_count;
}

uint16_t stunrun_gsp_combine_byte_lanes(uint8_t high_lane, uint8_t low_lane)
{
    return (uint16_t)(((uint16_t)high_lane << 8) | low_lane);
}

size_t stunrun_gsp_expand_byte_lane_streams(
    const uint8_t *base_lanes, const uint8_t *twin_lanes, size_t lane_count,
    uint16_t *base_words, uint16_t *twin_words, size_t word_capacity)
{
    size_t pairs;
    size_t i;

    if (base_lanes == NULL || twin_lanes == NULL ||
        base_words == NULL || twin_words == NULL ||
        lane_count < 2u || word_capacity == 0u)
        return 0u;
    pairs = lane_count / 2u;
    if (pairs > word_capacity)
        pairs = word_capacity;
    for (i = 0; i < pairs; i++) {
        base_words[i] = stunrun_gsp_combine_byte_lanes(
            base_lanes[i * 2u], base_lanes[i * 2u + 1u]);
        twin_words[i] = stunrun_gsp_combine_byte_lanes(
            twin_lanes[i * 2u], twin_lanes[i * 2u + 1u]);
    }
    return pairs;
}
