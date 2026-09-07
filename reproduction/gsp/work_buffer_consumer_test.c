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

    CHECK(stunrun_gsp_combine_byte_lanes(0x12u, 0x34u) == 0x1234u);
    CHECK(stunrun_gsp_combine_byte_lanes(0xFFu, 0x00u) == 0xFF00u);

    if (failures == 0)
        puts("gsp-work-buffer-consumer-slice: PASS");
    return failures != 0;
}
