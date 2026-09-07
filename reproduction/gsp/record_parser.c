/* Literal GSP indexed-record slice. */

#include "record_parser.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

int stunrun_gsp_record_address(uint32_t base, uint16_t index,
                               uint32_t *address)
{
    uint32_t offset = (uint32_t)index * STUNRUN_GSP_RECORD_BYTES;

    if (address == NULL || UINT32_MAX - base < offset)
        return 0;
    *address = base + offset;
    return 1;
}

void stunrun_gsp_record_load(const uint16_t *words,
                             stunrun_gsp_record_t *record)
{
    if (words == NULL || record == NULL)
        return;
    memcpy(record->raw, words, sizeof(record->raw));
}

static uint16_t word(const stunrun_gsp_record_t *record, size_t index)
{
    return record == NULL ? 0u : record->raw[index];
}

uint16_t stunrun_gsp_record_header(const stunrun_gsp_record_t *record)
{
    return word(record, 0u);
}

uint16_t stunrun_gsp_record_field_a8_initial(const stunrun_gsp_record_t *record)
{
    return word(record, 1u);
}

uint16_t stunrun_gsp_record_field_a6(const stunrun_gsp_record_t *record)
{
    return word(record, 2u);
}

uint16_t stunrun_gsp_record_field_a8_late(const stunrun_gsp_record_t *record)
{
    return word(record, 3u);
}

uint16_t stunrun_gsp_record_coord_a7(const stunrun_gsp_record_t *record)
{
    return word(record, 4u);
}

uint16_t stunrun_gsp_record_coord_a9(const stunrun_gsp_record_t *record)
{
    return word(record, 5u);
}

uint16_t stunrun_gsp_record_count_a10(const stunrun_gsp_record_t *record)
{
    return word(record, 6u);
}
