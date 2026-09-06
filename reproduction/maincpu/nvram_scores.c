/* See nvram_scores.h for provenance and confidence notes. */
#include "nvram_scores.h"

uint32_t stunrun_nvram_score_address(unsigned index)
{
    if (index >= STUNRUN_NVRAM_SCORE_COUNT)
        return 0;
    return STUNRUN_NVRAM_SCORE_BASE +
           index * STUNRUN_NVRAM_SCORE_RECORD_SIZE;
}

uint16_t stunrun_nvram_score_decode(
    const uint8_t record[STUNRUN_NVRAM_SCORE_RECORD_SIZE])
{
    if (record == NULL)
        return 0;
    return (uint16_t)(((uint16_t)record[STUNRUN_NVRAM_SCORE_FIELD_OFFSET]
                       << 8) |
                      record[STUNRUN_NVRAM_SCORE_FIELD_OFFSET + 1]);
}

const uint8_t *stunrun_nvram_name_bytes(
    const uint8_t record[STUNRUN_NVRAM_SCORE_RECORD_SIZE])
{
    if (record == NULL)
        return NULL;
    return record + STUNRUN_NVRAM_NAME_FIELD_OFFSET;
}
