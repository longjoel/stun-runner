/* Persistent NVRAM high-score table decoder, in C.
 *
 * Agent 2 (Implementer) encoding of Agent 1's frozen table finding.
 * Literal layout only.
 *
 * Provenance: reference/experiments/stunrun/main-nvram-high-score-table.
 * metadata.json (SNAPSHOT + PERSISTENT-NVRAM + STATIC): the 68010 ZRAM
 * view at 0xFF4000-0xFF4FFF combines the M48T02 high lane and the 2816
 * EEPROM low lane; the table at 0xFF4410-0xFF44FF holds ten 24-byte
 * records with a big-endian u16 score at +0 and display name bytes at +2.
 * The snapshot is identical at frames 1 and 600 and across input modes.
 *
 * Deliberately NOT claimed: the name bytes' termination/padding
 * convention is unestablished, so the decoder exposes the name field
 * address and the test asserts only the recorded name prefix (true under
 * any prefix-stored convention). Live-score-to-table promotion and the
 * game-over write event remain open.
 *
 * No ROM contents are embedded or required.
 */
#ifndef STUNRUN_NVRAM_SCORES_H
#define STUNRUN_NVRAM_SCORES_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ZRAM view and table layout (68010 view). */
#define STUNRUN_NVRAM_VIEW_BASE 0xFF4000u
#define STUNRUN_NVRAM_VIEW_LAST 0xFF4FFFu
#define STUNRUN_NVRAM_SCORE_BASE 0xFF4410u
#define STUNRUN_NVRAM_SCORE_RECORD_SIZE 24u
#define STUNRUN_NVRAM_SCORE_COUNT 10u
#define STUNRUN_NVRAM_SCORE_FIELD_OFFSET 0u
#define STUNRUN_NVRAM_NAME_FIELD_OFFSET 2u
#define STUNRUN_NVRAM_NAME_FIELD_SIZE \
    (STUNRUN_NVRAM_SCORE_RECORD_SIZE - STUNRUN_NVRAM_NAME_FIELD_OFFSET)

/* Base address of record i, or 0 for i >= COUNT. */
uint32_t stunrun_nvram_score_address(unsigned index);

/* Big-endian u16 score at record offset +0. NULL record reads as 0. */
uint16_t stunrun_nvram_score_decode(const uint8_t record[STUNRUN_NVRAM_SCORE_RECORD_SIZE]);

/* Address of the display-name bytes (record offset +2). NULL in, NULL out. */
const uint8_t *stunrun_nvram_name_bytes(
    const uint8_t record[STUNRUN_NVRAM_SCORE_RECORD_SIZE]);

#ifdef __cplusplus
}
#endif

#endif /* STUNRUN_NVRAM_SCORES_H */
