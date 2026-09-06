/* ROM-free self-check for the NVRAM high-score decoder.
 *
 * Golden vectors transcribed from main-nvram-high-score-table.metadata.
 * json: ten (address, score, name) entries. Score bytes are written
 * explicitly big-endian per the recorded layout; name padding uses 0xAA
 * filler to prove the test never depends on the unestablished
 * termination/padding convention — only the recorded name prefix is
 * asserted. Exit 0 on success.
 */
#include <stdio.h>
#include <string.h>

#include "nvram_scores.h"

static int g_failures = 0;

#define check(cond) \
    do { \
        if (cond) { \
            printf("PASS %s\n", #cond); \
        } else { \
            printf("FAIL %s\n", #cond); \
            g_failures++; \
        } \
    } while (0)

typedef struct golden_entry {
    uint32_t address;
    uint8_t score_hi;
    uint8_t score_lo;
    const char *name;
} golden_entry_t;

/* Explicit BE bytes: 15000=0x3A98, 12500=0x30D4, 10000=0x2710,
 * 8000=0x1F40, 7000=0x1B58, 5000=0x1388, 4000=0x0FA0, 3000=0x0BB8,
 * 2000=0x07D0, 1000=0x03E8. */
static const golden_entry_t kGolden[] = {
    { 0xFF4410u, 0x3Au, 0x98u, "THE GONZ" },
    { 0xFF4428u, 0x30u, 0xD4u, "SMOKY" },
    { 0xFF4440u, 0x27u, 0x10u, "GUNNER GLENN" },
    { 0xFF4458u, 0x1Fu, 0x40u, "BAD BABE" },
    { 0xFF4470u, 0x1Bu, 0x58u, "THE POTATOE" },
    { 0xFF4488u, 0x13u, 0x88u, "SCOOTER" },
    { 0xFF44A0u, 0x0Fu, 0xA0u, "THE HOOPLE" },
    { 0xFF44B8u, 0x0Bu, 0xB8u, "RANGER RICK" },
    { 0xFF44D0u, 0x07u, 0xD0u, "BUGS" },
    { 0xFF44E8u, 0x03u, 0xE8u, "POGO" },
};

int main(void)
{
    unsigned i;
    check(STUNRUN_NVRAM_SCORE_COUNT == 10u);
    check(STUNRUN_NVRAM_SCORE_RECORD_SIZE == 24u);

    for (i = 0; i < STUNRUN_NVRAM_SCORE_COUNT; i++) {
        uint8_t record[STUNRUN_NVRAM_SCORE_RECORD_SIZE];
        size_t name_len = strlen(kGolden[i].name);
        const uint8_t *name;
        /* Address stride reproduces the recorded table addresses. */
        if (stunrun_nvram_score_address(i) != kGolden[i].address) {
            printf("FAIL address entry %u: got 0x%08X want 0x%08X\n", i,
                   stunrun_nvram_score_address(i), kGolden[i].address);
            g_failures++;
            continue;
        }
        printf("PASS address entry %u\n", i);
        /* Fixture: explicit score bytes, ASCII name, 0xAA padding that
         * no assertion may depend on. */
        memset(record, 0xAA, sizeof(record));
        record[0] = kGolden[i].score_hi;
        record[1] = kGolden[i].score_lo;
        memcpy(record + STUNRUN_NVRAM_NAME_FIELD_OFFSET, kGolden[i].name,
               name_len);
        check(stunrun_nvram_score_decode(record) ==
              (uint16_t)(((uint16_t)kGolden[i].score_hi << 8) |
                         kGolden[i].score_lo));
        name = stunrun_nvram_name_bytes(record);
        check(name == record + STUNRUN_NVRAM_NAME_FIELD_OFFSET);
        check(memcmp(name, kGolden[i].name, name_len) == 0);
    }

    /* Table bounds: last record ends at 0xFF4500, inside the ZRAM view;
     * out-of-range index reads as 0. */
    check(stunrun_nvram_score_address(9u) + STUNRUN_NVRAM_SCORE_RECORD_SIZE ==
          0xFF4500u);
    check(0xFF4500u <= STUNRUN_NVRAM_VIEW_LAST + 1u);
    check(stunrun_nvram_score_address(10u) == 0u);

    /* NULL record safety. */
    check(stunrun_nvram_score_decode(NULL) == 0u);
    check(stunrun_nvram_name_bytes(NULL) == NULL);

    if (g_failures == 0)
        printf("nvram scores C port: all checks passed\n");
    return g_failures == 0 ? 0 : 1;
}
