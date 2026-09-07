/* Literal GSP indexed-record addressing and field extraction.
 *
 * Provenance: M5 runtime traces at GSP PCs 0xFFF45000, 0xFFF45090,
 * 0xFFF45890, 0xFFF45A10, 0xFFF45A40, and 0xFFF45A50.
 *
 * The code preserves raw words. It does not assign road/object/HUD meaning.
 */
#ifndef STUNRUN_GSP_RECORD_PARSER_H
#define STUNRUN_GSP_RECORD_PARSER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STUNRUN_GSP_RECORD_WORDS 8u
#define STUNRUN_GSP_RECORD_BYTES 16u

typedef struct stunrun_gsp_record {
    uint16_t raw[STUNRUN_GSP_RECORD_WORDS];
} stunrun_gsp_record_t;

/* Literal FFF45000 address calculation: base + (index << 4). */
int stunrun_gsp_record_address(uint32_t base, uint16_t index,
                               uint32_t *address);

/* Copy one observed 16-bit record without transforming its fields. */
void stunrun_gsp_record_load(const uint16_t *words,
                             stunrun_gsp_record_t *record);

/* Literal consumers in the traced path. */
uint16_t stunrun_gsp_record_header(const stunrun_gsp_record_t *record);
uint16_t stunrun_gsp_record_field_a8_initial(const stunrun_gsp_record_t *record);
uint16_t stunrun_gsp_record_field_a6(const stunrun_gsp_record_t *record);
uint16_t stunrun_gsp_record_field_a8_late(const stunrun_gsp_record_t *record);
uint16_t stunrun_gsp_record_coord_a7(const stunrun_gsp_record_t *record);
uint16_t stunrun_gsp_record_coord_a9(const stunrun_gsp_record_t *record);
uint16_t stunrun_gsp_record_count_a10(const stunrun_gsp_record_t *record);

#ifdef __cplusplus
}
#endif

#endif
