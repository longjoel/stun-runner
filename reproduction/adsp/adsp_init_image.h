/* ADSP-2100 replacement init-slice image, C port.
 *
 * Agent 2 (Implementer) port of the M3 source-defined fixtures in
 * tools/build-replacement-image. Literal mechanism only: every word value
 * below is the 24-bit ADSP program word already proven by the Python builder
 * and, for the setup/routine bodies, decoded from the original frame-600
 * image. See analysis/m3-adsp-reset-entry.md for provenance.
 *
 * No ROM contents are embedded or required. The reset entry at program word
 * 0x0004 is MAME-confirmed (STATUS.md M3 record).
 *
 * Confidence of the encoded words: OBSERVED-IN-TRACE (reset calls) and
 * literal decoded setup (init-state bodies). Control-flow meaning beyond
 * the stated loop targets is UNKNOWN.
 */
#ifndef STUNRUN_ADSP_INIT_IMAGE_H
#define STUNRUN_ADSP_INIT_IMAGE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Fixed-origin layout shared by every fixture in this slice. */
#define STUNRUN_ADSP_ORIGIN 0x0000u
#define STUNRUN_ADSP_ENTRY 0x0004u

/* Word counts (one 24-bit word per entry, held in a uint32_t). */
#define STUNRUN_ADSP_NOP_WORDS 5u
#define STUNRUN_ADSP_RESET_LOOP_WORDS 5u
#define STUNRUN_ADSP_INIT_PREFIX_WORDS 0x835u
#define STUNRUN_ADSP_INIT_STATE_WORDS 0x847u

/* Independently derived encodings (MAME 2100 executor/dasm rules). */
#define STUNRUN_ADSP_CALL 0x001C0000u /* OR target<<4 | cond; always cond=0xf */
#define STUNRUN_ADSP_JUMP 0x00180000u
#define STUNRUN_ADSP_RTS 0x000A000Fu
#define STUNRUN_ADSP_COND_ALWAYS 0xFu

/* Observed reset-path call targets (analysis/m3-adsp-reset-entry.md). */
#define STUNRUN_ADSP_SUB_0780 0x0780u
#define STUNRUN_ADSP_SUB_0834 0x0834u

/* Mailbox-boundary loop target for the init-state slice. */
#define STUNRUN_ADSP_MAILBOX_BOUNDARY 0x0050u

typedef enum stunrun_adsp_fixture {
    STUNRUN_ADSP_FIXTURE_NOP = 0,
    STUNRUN_ADSP_FIXTURE_RESET_LOOP,
    STUNRUN_ADSP_FIXTURE_INIT_PREFIX,
    STUNRUN_ADSP_FIXTURE_INIT_STATE
} stunrun_adsp_fixture_t;

/* Number of 24-bit words for a fixture, or 0 for an unknown fixture. */
size_t stunrun_adsp_fixture_words(stunrun_adsp_fixture_t fixture);

/* Fill the first `count` image words (big-endian 32-bit containers holding
 * 24-bit values) for a fixture. Words past the fixture length read as 0.
 * Returns the number of words written, or 0 for an unknown fixture. */
size_t stunrun_adsp_emit(stunrun_adsp_fixture_t fixture,
                         uint32_t *out, size_t count);

/* Observed DM landmarks reproduced by the init-state slice
 * (analysis/ram-map.md: OBSERVED-IN-REPLACEMENT). */
#define STUNRUN_ADSP_DM_SIZE 0x2000u
typedef struct stunrun_adsp_dm_landmark {
    uint16_t addr;
    uint16_t value;
} stunrun_adsp_dm_landmark_t;

#define STUNRUN_ADSP_DM_LANDMARK_COUNT 4u

/* The four landmark (address, value) pairs, in increasing address order. */
const stunrun_adsp_dm_landmark_t *stunrun_adsp_dm_landmarks(void);

/* Apply the landmarks to a 0x2000-entry DM array. No-op on NULL. */
void stunrun_adsp_apply_dm_landmarks(uint16_t dm[STUNRUN_ADSP_DM_SIZE]);

#ifdef __cplusplus
}
#endif

#endif /* STUNRUN_ADSP_INIT_IMAGE_H */
