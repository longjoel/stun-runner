/* Literal 8-word text record feeding the GSP PIXBLT cursor. */
#ifndef STUNRUN_GSP_TEXT_RECORD_H
#define STUNRUN_GSP_TEXT_RECORD_H

#include <stdint.h>

#include "text_cursor.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STUNRUN_GSP_TEXT_RECORD_WORDS 8u
#define STUNRUN_GSP_TEXT_RECORD_DESCRIPTOR_WORD 4u
#define STUNRUN_GSP_TEXT_RECORD_WORD_STRIDE 0x10u

typedef struct stunrun_gsp_text_record {
    uint16_t raw[STUNRUN_GSP_TEXT_RECORD_WORDS];
} stunrun_gsp_text_record_t;

void stunrun_gsp_text_record_load(const uint16_t *words,
                                  stunrun_gsp_text_record_t *record);
uint16_t stunrun_gsp_text_record_header(
    const stunrun_gsp_text_record_t *record);
uint16_t stunrun_gsp_text_record_x(const stunrun_gsp_text_record_t *record);
uint16_t stunrun_gsp_text_record_y(const stunrun_gsp_text_record_t *record);

/* Convert the literal record base/coordinate fields into the cursor seeds
 * observed by the descriptor reader. The packed words remain in record. */
int stunrun_gsp_text_record_cursor(
    uint32_t record_address, const stunrun_gsp_text_record_t *record,
    uint32_t *descriptor_address, uint32_t *a1,
    const uint16_t **packed_words);

#ifdef __cplusplus
}
#endif

#endif
