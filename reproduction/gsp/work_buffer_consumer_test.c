/* ROM-free self-check for the literal GSP FIFO consumer setup. */
#include "work_buffer_consumer.h"

#include <stdio.h>

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", #condition); \
        failures++; \
    } \
} while (0)

int main(void)
{
    uint32_t base = 0u;
    uint32_t twin = 0u;

    CHECK(stunrun_gsp_work_buffer_addresses(0x1201u, &base, &twin));
    CHECK(base == STUNRUN_GSP_WORK_BUFFER_BASE + 0x10u);
    CHECK(twin == STUNRUN_GSP_WORK_BUFFER_TWIN + 0x10u);
    CHECK(stunrun_gsp_work_buffer_addresses(0x12FFu, &base, &twin));
    CHECK(base == STUNRUN_GSP_WORK_BUFFER_BASE + 0xFF0u);
    CHECK(twin == STUNRUN_GSP_WORK_BUFFER_TWIN + 0xFF0u);
    CHECK(!stunrun_gsp_work_buffer_addresses(0u, NULL, &twin));
    CHECK(!stunrun_gsp_work_buffer_addresses(0u, &base, NULL));
    CHECK(stunrun_gsp_work_buffer_write_addresses(0u, &base, &twin));
    CHECK(base == STUNRUN_GSP_WORK_BUFFER_WRITE_BASE);
    CHECK(twin == STUNRUN_GSP_WORK_BUFFER_WRITE_TWIN);
    CHECK(stunrun_gsp_work_buffer_write_addresses(127u, &base, &twin));
    CHECK(base == 0xFFF70630u && twin == 0xFFF71640u);
    CHECK(!stunrun_gsp_work_buffer_write_addresses(128u, &base, &twin));
    CHECK(!stunrun_gsp_work_buffer_write_addresses(0u, NULL, &twin));
    {
        const uint16_t base_values[] = {0xBD08u, 0x0102u};
        const uint16_t twin_values[] = {0x6565u, 0x0304u};
        stunrun_gsp_work_buffer_write_t base_events[2];
        stunrun_gsp_work_buffer_write_t twin_events[2];
        CHECK(stunrun_gsp_work_buffer_make_write_events(
                  base_values, twin_values, 2u, base_events, 2u,
                  twin_events, 2u) == 2u);
        CHECK(base_events[0].address == 0xFFF6F650u &&
              base_events[0].value == 0xBD08u);
        CHECK(twin_events[0].address == 0xFFF70660u &&
              twin_events[0].value == 0x6565u);
        CHECK(base_events[1].address == 0xFFF6F670u &&
              twin_events[1].address == 0xFFF70680u);
        CHECK(stunrun_gsp_work_buffer_make_write_events(
                  base_values, twin_values, 2u, base_events, 1u,
                  twin_events, 2u) == 0u);
        CHECK(stunrun_gsp_work_buffer_make_write_events(
                  base_values, twin_values, 129u, base_events, 2u,
                  twin_events, 2u) == 0u);
    }

    CHECK(stunrun_gsp_combine_byte_lanes(0x12u, 0x34u) == 0x1234u);
    CHECK(stunrun_gsp_combine_byte_lanes(0xFFu, 0x00u) == 0xFF00u);
    {
        const uint8_t base_lanes[] = {0x01u, 0x23u, 0x45u, 0x67u, 0x89u};
        const uint8_t twin_lanes[] = {0xBDu, 0x08u, 0x65u, 0x65u, 0xAAu};
        uint16_t base_words[2] = {0u, 0u};
        uint16_t twin_words[2] = {0u, 0u};
        CHECK(stunrun_gsp_expand_byte_lane_streams(
                  base_lanes, twin_lanes, sizeof(base_lanes), base_words,
                  twin_words, 2u) == 2u);
        CHECK(base_words[0] == 0x0123u && base_words[1] == 0x4567u);
        CHECK(twin_words[0] == 0xBD08u && twin_words[1] == 0x6565u);
        CHECK(stunrun_gsp_expand_byte_lane_streams(
                  base_lanes, twin_lanes, sizeof(base_lanes), base_words,
                  twin_words, 1u) == 1u);
        CHECK(stunrun_gsp_expand_byte_lane_streams(
                  base_lanes, twin_lanes, 1u, base_words, twin_words, 2u) ==
              0u);
        CHECK(stunrun_gsp_expand_byte_lane_streams(
                  base_lanes, NULL, sizeof(base_lanes), base_words,
                  twin_words, 2u) == 0u);
    }

    if (failures == 0)
        puts("gsp-work-buffer-consumer-slice: PASS");
    return failures != 0;
}
