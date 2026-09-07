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

uint16_t stunrun_gsp_combine_byte_lanes(uint8_t high_lane, uint8_t low_lane)
{
    return (uint16_t)(((uint16_t)high_lane << 8) | low_lane);
}
