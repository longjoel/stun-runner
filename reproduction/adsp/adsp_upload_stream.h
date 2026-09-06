/* 68010-side ADSP program-upload stream contract, in C.
 *
 * Agent 2 (Implementer) encoding of Agent 1's M1 upload findings: the
 * 68010 uploader that populates ADSP program RAM over the title path.
 * Literal framing only — the record PAYLOAD semantics remain unresolved
 * (see adsp-program-upload.metadata.json) and nothing here claims them.
 *
 * Provenance: reference/experiments/stunrun/adsp-program-upload.
 * metadata.json (OBSERVED-IN-TRACE in two independent 600-frame runs,
 * no-input and coin/start identical) plus the pinned driver_68k_map
 * (MAME-CONFIRMED ROM range for the source pointer).
 *
 * Observed mechanism, both runs:
 * - caller at 0x02C204 reads the 32-bit source pointer 0x0001702E from
 *   $17000 and passes it to 0x02D2E0; the pointer lies in 68010 ROM
 *   (0x000000-0x0FFFFF) and the source window shows no writes, so the
 *   stream is ROM-resident;
 * - the transfer loop at 0x02D304-0x02D364 uploads through the single
 *   writer PC 0x02D35C (`move.l D0,(A2)+`), producing 5456 halfword taps
 *   over 0x800000-0x8048D6, i.e. 2728 MOVE.L transfers;
 * - normalizing the source bytes by 68010 lane masks yields 17 records
 *   (control 0) totaling 2728 24-bit program words, one explicit
 *   destination gap (word indices 2203-4136), then a 0xFF terminator.
 *
 * Address rule derived from the observed coverage: destination word index
 * i is written by one MOVE.L at byte address 0x800000 + 4*i, so the final
 * record (words 4150-4661) ends with the halfword tap at 0x8048D6. This
 * reproduces the observed range exactly; it does not explain WHY the
 * 24-bit words travel in 32-bit containers.
 *
 * No ROM contents are embedded or required.
 */
#ifndef STUNRUN_ADSP_UPLOAD_STREAM_H
#define STUNRUN_ADSP_UPLOAD_STREAM_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Uploader PCs (static listing + observed writer). */
#define STUNRUN_ADSP_UPLOAD_READER_PC 0x02C204u
#define STUNRUN_ADSP_UPLOAD_CALLEE_PC 0x02D2E0u
#define STUNRUN_ADSP_UPLOAD_WRITER_PC 0x02D35Cu
#define STUNRUN_ADSP_UPLOAD_LOOP_FIRST 0x02D304u
#define STUNRUN_ADSP_UPLOAD_LOOP_LAST 0x02D364u

/* ROM-resident source stream. */
#define STUNRUN_ADSP_UPLOAD_SOURCE_PTR 0x0001702Eu
#define STUNRUN_ADSP_UPLOAD_SOURCE_ADDR 0x0017000u
#define STUNRUN_ADSP_UPLOAD_ROM_FIRST 0x000000u
#define STUNRUN_ADSP_UPLOAD_ROM_LAST 0x0FFFFFu

/* 68010-visible ADSP program window and observed title-path coverage. */
#define STUNRUN_ADSP_UPLOAD_WINDOW_BASE 0x800000u
#define STUNRUN_ADSP_UPLOAD_WINDOW_LAST 0x807FFFu
#define STUNRUN_ADSP_UPLOAD_OBSERVED_FIRST 0x800000u
#define STUNRUN_ADSP_UPLOAD_OBSERVED_LAST 0x8048D6u

/* Transfer totals: 2728 MOVE.L transfers == 5456 halfword taps. */
#define STUNRUN_ADSP_UPLOAD_PROGRAM_WORDS 2728u
#define STUNRUN_ADSP_UPLOAD_HALFWORD_TAPS 5456u

/* Record framing. */
#define STUNRUN_ADSP_UPLOAD_RECORD_COUNT 17u
#define STUNRUN_ADSP_UPLOAD_CONTROL_DATA 0x00u
#define STUNRUN_ADSP_UPLOAD_CONTROL_TERMINATOR 0xFFu

/* Explicit destination gap (word indices, inclusive). */
#define STUNRUN_ADSP_UPLOAD_GAP_FIRST 2203u
#define STUNRUN_ADSP_UPLOAD_GAP_LAST 4136u
#define STUNRUN_ADSP_UPLOAD_GAP_WORDS 1934u

/* One decoded record: control is always 0; the stream ends with the 0xFF
 * terminator after the final payload. */
typedef struct stunrun_adsp_upload_record {
    uint16_t destination_word_index;
    uint16_t program_word_count;
} stunrun_adsp_upload_record_t;

size_t stunrun_adsp_upload_record_count(void);
const stunrun_adsp_upload_record_t *stunrun_adsp_upload_records(void);

/* Total 24-bit program words across all records (2728). */
unsigned stunrun_adsp_upload_total_words(void);

/* MOVE.L byte address for destination word index i:
 * 0x800000 + 4*i. Returns 0 when i falls outside the program window. */
uint32_t stunrun_adsp_upload_word_address(unsigned word_index);

/* Nonzero when word index i lies inside the explicit gap. */
int stunrun_adsp_upload_in_gap(unsigned word_index);

/* Record index containing word index i, or -1 for gap/uncovered words. */
int stunrun_adsp_upload_record_for_word(unsigned word_index);

/* High/low bus halfwords of the 32-bit source pointer, as observed on
 * the 68010 bus (0x0001, 0x702E). */
uint16_t stunrun_adsp_upload_source_ptr_hi(void);
uint16_t stunrun_adsp_upload_source_ptr_lo(void);

#ifdef __cplusplus
}
#endif

#endif /* STUNRUN_ADSP_UPLOAD_STREAM_H */
