/* 68010-visible ADSP control/IRQ contract and install boundary, in C.
 *
 * Agent 2 (Implementer) encoding of Agent 1's M1/M3 control-window findings
 * for M3 objective items 1-2: the earliest bounded install point and the
 * verified ADSP reset/control sequence. Literal mechanism only.
 *
 * Provenance:
 * - Control/IRQ table: reference/experiments/stunrun/adsp-control-window.
 *   metadata.json (MAME confirms handler semantics via harddriv_m.cpp:685-
 *   765; the per-address counts and all-zero data are OBSERVED-IN-TRACE in
 *   two independent 600-frame runs, no-input and coin/start identical).
 * - Upload/install frames: reference/experiments/stunrun/m3-adsp-upload-
 *   boundary.metadata.json (program RAM empty through frame 407, first
 *   populated at 408, complete 2718-word image at 411); the frame-412
 *   install point is the init-prefix experiment record in
 *   analysis/m3-adsp-reset-entry.md.
 *
 * What this deliberately does NOT claim: the intra-run ORDER of the 73
 * control writes was not recorded, so the contract is a multiset (counts
 * per address), not a sequence. Assert-before-release pairing and any
 * post-title-path traffic are UNKNOWN. Comparing a replacement run
 * against the canonical title checkpoint (M3 item 3) is Verifier-owned;
 * this file only supplies the reusable tally the comparison consumes.
 *
 * No ROM contents are embedded or required.
 */
#ifndef STUNRUN_ADSP_CONTROL_SEQ_H
#define STUNRUN_ADSP_CONTROL_SEQ_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* One 68010-visible ADSP control-window operation. `expected_writes` is the
 * count observed in each independent 600-frame title-path run; every
 * observed write carried data 0x0000. */
typedef struct stunrun_adsp_control_op {
    uint32_t address;
    unsigned word_offset;
    const char *handler;
    unsigned expected_writes;
} stunrun_adsp_control_op_t;

#define STUNRUN_ADSP_CONTROL_OP_COUNT 11u
#define STUNRUN_ADSP_CONTROL_TOTAL_WRITES 73u
#define STUNRUN_ADSP_CONTROL_DATA 0x0000u

size_t stunrun_adsp_control_op_count(void);
const stunrun_adsp_control_op_t *stunrun_adsp_control_ops(void);

/* Index of the op for a control-window address, or -1 when unknown. */
int stunrun_adsp_control_lookup(uint32_t address);

/* IRQ-clear contract: two repeatable writes per title-path run. */
#define STUNRUN_ADSP_IRQ_CLEAR_ADDR 0x818060u
#define STUNRUN_ADSP_IRQ_CLEAR_WRITES 2u
#define STUNRUN_ADSP_IRQ_CLEAR_PC_COUNT 2u

/* Writer PCs of the IRQ-clear pair, in no required order. */
const uint32_t *stunrun_adsp_irq_clear_pcs(void);

/* Nonzero when pc is one of the two observed IRQ-clear writer PCs. */
int stunrun_adsp_irq_clear_pc_known(uint32_t pc);

/* IRQ-state window: the bounded title path performs zero reads here.
 * Whether post-gameplay traffic exists is UNKNOWN. */
#define STUNRUN_ADSP_IRQ_STATE_BASE 0x838000u
#define STUNRUN_ADSP_IRQ_STATE_LAST 0x83FFFFu
#define STUNRUN_ADSP_IRQ_STATE_TITLE_READS 0u

/* Upload/install frame boundary (M3 item 1). */
#define STUNRUN_ADSP_UPLOAD_LAST_ZERO_FRAME 407u
#define STUNRUN_ADSP_UPLOAD_FIRST_POPULATED_FRAME 408u
#define STUNRUN_ADSP_UPLOAD_FIRST_POPULATED_WORDS 293u
#define STUNRUN_ADSP_UPLOAD_COMPLETE_FRAME 411u
#define STUNRUN_ADSP_UPLOAD_COMPLETE_WORDS 2718u
#define STUNRUN_ADSP_REPLACEMENT_INSTALL_FRAME 412u

typedef enum stunrun_adsp_upload_state {
    STUNRUN_ADSP_UPLOAD_EMPTY = 0,   /* program RAM still all zero */
    STUNRUN_ADSP_UPLOAD_POPULATING,  /* transition window 408-410 */
    STUNRUN_ADSP_UPLOAD_COMPLETE     /* full image present */
} stunrun_adsp_upload_state_t;

/* Bounded upload state of ADSP program RAM at a title-path frame. */
stunrun_adsp_upload_state_t stunrun_adsp_upload_state_at(unsigned frame);

/* Nonzero once the original upload is complete and the replacement image
 * may be installed without relying on it (frame >= 412). */
int stunrun_adsp_install_frame_ready(unsigned frame);

/* Multiset tally of observed control-window writes for one bounded run.
 * Feed every observed (address, data) pair with tally_add, then ask
 * tally_matches whether the run reproduced the title-path contract. */
typedef struct stunrun_adsp_control_tally {
    unsigned per_op[STUNRUN_ADSP_CONTROL_OP_COUNT];
    unsigned unknown_addresses;
    unsigned data_mismatches;
} stunrun_adsp_control_tally_t;

void stunrun_adsp_control_tally_init(stunrun_adsp_control_tally_t *tally);

/* Returns the op index, or -1 for an unknown address. NULL tally is a
 * no-op returning -1. A write whose data differs from 0x0000 still counts
 * toward its op and additionally records a data mismatch. */
int stunrun_adsp_control_tally_add(stunrun_adsp_control_tally_t *tally,
                                   uint32_t address, unsigned data);

/* Nonzero iff every op count equals its expectation with no unknown
 * addresses and no data mismatches. */
int stunrun_adsp_control_tally_matches(
    const stunrun_adsp_control_tally_t *tally);

#ifdef __cplusplus
}
#endif

#endif /* STUNRUN_ADSP_CONTROL_SEQ_H */
