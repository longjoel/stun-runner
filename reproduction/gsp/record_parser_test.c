#include "record_parser.h"

#include <stdint.h>
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
    const uint16_t words[STUNRUN_GSP_RECORD_WORDS] = {
        0x0100u, 0x0101u, 0x0102u, 0x0103u,
        0x0104u, 0x0105u, 0x0106u, 0x0107u
    };
    stunrun_gsp_record_t record;
    uint32_t address = 0u;

    CHECK(stunrun_gsp_record_address(0x1000u, 0x11u, &address));
    CHECK(address == 0x1110u);
    CHECK(!stunrun_gsp_record_address(UINT32_MAX - 1u, 1u, &address));
    CHECK(!stunrun_gsp_record_address(0u, 0u, NULL));

    stunrun_gsp_record_load(words, &record);
    CHECK(stunrun_gsp_record_header(&record) == 0x0100u);
    CHECK(stunrun_gsp_record_field_a8_initial(&record) == 0x0101u);
    CHECK(stunrun_gsp_record_field_a6(&record) == 0x0102u);
    CHECK(stunrun_gsp_record_field_a8_late(&record) == 0x0103u);
    CHECK(stunrun_gsp_record_coord_a7(&record) == 0x0104u);
    CHECK(stunrun_gsp_record_coord_a9(&record) == 0x0105u);
    CHECK(stunrun_gsp_record_count_a10(&record) == 0x0106u);
    CHECK(stunrun_gsp_record_header(NULL) == 0u);

    if (failures != 0)
        return 1;
    puts("record-parser-slice: PASS");
    return 0;
}
