/* See adsp_upload_stream.h for provenance and confidence notes. */
#include "adsp_upload_stream.h"

/* Exact decoded_stream_contract.records table from
 * adsp-program-upload.metadata.json. */
static const stunrun_adsp_upload_record_t kRecords[] = {
    { 0, 246 }, { 246, 30 }, { 276, 96 }, { 372, 160 },
    { 532, 72 }, { 604, 598 }, { 1202, 57 }, { 1259, 242 },
    { 1501, 84 }, { 1585, 50 }, { 1635, 62 }, { 1697, 223 },
    { 1920, 139 }, { 2059, 41 }, { 2100, 103 }, { 4137, 13 },
    { 4150, 512 },
};

size_t stunrun_adsp_upload_record_count(void)
{
    return STUNRUN_ADSP_UPLOAD_RECORD_COUNT;
}

const stunrun_adsp_upload_record_t *stunrun_adsp_upload_records(void)
{
    return kRecords;
}

unsigned stunrun_adsp_upload_total_words(void)
{
    unsigned total = 0;
    size_t i;
    for (i = 0; i < STUNRUN_ADSP_UPLOAD_RECORD_COUNT; i++)
        total += kRecords[i].program_word_count;
    return total;
}

uint32_t stunrun_adsp_upload_word_address(unsigned word_index)
{
    uint32_t address;
    if (word_index > 0x1FFFu)
        return 0;
    address = STUNRUN_ADSP_UPLOAD_WINDOW_BASE + 4u * word_index;
    if (address < STUNRUN_ADSP_UPLOAD_WINDOW_BASE ||
        address + 3u > STUNRUN_ADSP_UPLOAD_WINDOW_LAST)
        return 0;
    return address;
}

int stunrun_adsp_upload_in_gap(unsigned word_index)
{
    return word_index >= STUNRUN_ADSP_UPLOAD_GAP_FIRST &&
           word_index <= STUNRUN_ADSP_UPLOAD_GAP_LAST;
}

int stunrun_adsp_upload_record_for_word(unsigned word_index)
{
    size_t i;
    for (i = 0; i < STUNRUN_ADSP_UPLOAD_RECORD_COUNT; i++) {
        unsigned first = kRecords[i].destination_word_index;
        unsigned last = first + kRecords[i].program_word_count - 1u;
        if (word_index >= first && word_index <= last)
            return (int)i;
    }
    return -1;
}

uint16_t stunrun_adsp_upload_source_ptr_hi(void)
{
    return (uint16_t)(STUNRUN_ADSP_UPLOAD_SOURCE_PTR >> 16);
}

uint16_t stunrun_adsp_upload_source_ptr_lo(void)
{
    return (uint16_t)(STUNRUN_ADSP_UPLOAD_SOURCE_PTR & 0xFFFFu);
}
