/* Literal GSP text-record to PIXBLT cursor setup. */

#include "text_record.h"

#include <stddef.h>
#include <string.h>

void stunrun_gsp_text_record_load(const uint16_t *words,
                                  stunrun_gsp_text_record_t *record)
{
    if (words == NULL || record == NULL)
        return;
    memcpy(record->raw, words, sizeof(record->raw));
}

static uint16_t word(const stunrun_gsp_text_record_t *record, size_t index)
{
    return record == NULL ? 0u : record->raw[index];
}

uint16_t stunrun_gsp_text_record_header(
    const stunrun_gsp_text_record_t *record)
{
    return word(record, 0u);
}

uint16_t stunrun_gsp_text_record_x(const stunrun_gsp_text_record_t *record)
{
    return word(record, 2u);
}

uint16_t stunrun_gsp_text_record_y(const stunrun_gsp_text_record_t *record)
{
    return word(record, 3u);
}

int stunrun_gsp_text_record_cursor(
    uint32_t record_address, const stunrun_gsp_text_record_t *record,
    uint32_t *descriptor_address, uint32_t *a1,
    const uint16_t **packed_words)
{
    uint32_t offset = STUNRUN_GSP_TEXT_RECORD_DESCRIPTOR_WORD *
                      STUNRUN_GSP_TEXT_RECORD_WORD_STRIDE;
    if (record == NULL || descriptor_address == NULL || a1 == NULL ||
        packed_words == NULL)
        return 0;
    *descriptor_address = record_address + offset;
    *a1 = ((uint32_t)stunrun_gsp_text_record_y(record) << 16) |
          stunrun_gsp_text_record_x(record);
    *packed_words = record->raw + STUNRUN_GSP_TEXT_RECORD_DESCRIPTOR_WORD;
    return 1;
}
