#include "text_record.h"

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
    static const uint16_t words[] = {
        0x4013u, 0x0000u, 0x0040u, 0x00FDu,
        0x3A30u, 0x3533u, 0x302Eu, 0x0000u
    };
    stunrun_gsp_text_record_t record;
    const uint16_t *packed = NULL;
    uint32_t descriptor = 0u;
    uint32_t a1 = 0u;

    stunrun_gsp_text_record_load(words, &record);
    CHECK(stunrun_gsp_text_record_header(&record) == 0x4013u);
    CHECK(stunrun_gsp_text_record_x(&record) == 0x40u);
    CHECK(stunrun_gsp_text_record_y(&record) == 0xFDu);
    CHECK(stunrun_gsp_text_record_cursor(
              0xFFFEA480u, &record, &descriptor, &a1, &packed));
    CHECK(descriptor == 0xFFFEA4C0u);
    CHECK(a1 == 0x00FD0040u);
    CHECK(packed == record.raw + 4u && packed[0] == 0x3A30u);
    CHECK(!stunrun_gsp_text_record_cursor(0u, NULL, &descriptor, &a1,
                                          &packed));
    if (failures != 0)
        return 1;
    puts("gsp-text-record-slice: PASS");
    return 0;
}
