/* ADSP-buffer to GSP-FIFO block transfer, in C.
 *
 * Agent 2 (Implementer) encoding of Agent 1's frozen block contract.
 * Literal mechanism only — block PAYLOAD semantics remain unresolved.
 *
 * Provenance:
 * - Block framing (106 count/0xFFFF blocks per 1200-frame run, count read
 *   at 0x810000 by PC 0x02F0C2, terminator 0xFFFF at 0x810000+count*2
 *   read by PC 0x02F0E2, loop PCs 0x02248E/0x02249A, 625989 reads over
 *   0x810000-0x813F50, both modes identical):
 *   reference/experiments/stunrun/adsp-buffer-window.metadata.json
 *   (OBSERVED-IN-TRACE).
 * - Transfer shape (caller 0x02C5EA passes 0x810000 to 0x02F0AE, 14
 *   executions per 600-frame run; callee reads the count, 0x02247A
 *   copies to the GSP FIFO at 0xC0000C with exactly one write per count
 *   unit, then checks the final word for -1):
 *   reference/experiments/stunrun/adsp-interface-map.metadata.json
 *   (OBSERVED-IN-TRACE).
 *
 * Deliberately NOT claimed: the evidence constrains the transfer only by
 * counts — C FIFO writes per count-C block, terminator at base+2*C. The
 * prose says the callee "advances past" the count word, which would put
 * the terminator at base+2+2*C and contradict the tap-observed formula;
 * whether the C FIFO'd units include the count word is therefore UNKNOWN
 * and the model takes an explicit payload of C units rather than
 * re-deriving it from the count. ADSP producer-side protocol and per-
 * block payload meaning are unresolved (see the metadata's next
 * questions). No buffer writes were observed in the title path.
 *
 * No ROM contents are embedded or required.
 */
#ifndef STUNRUN_FIFO_BLOCK_H
#define STUNRUN_FIFO_BLOCK_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Observed window, sink, and terminator. */
#define STUNRUN_FIFO_SOURCE_BASE 0x810000u
#define STUNRUN_FIFO_WINDOW_LAST 0x813FFFu
#define STUNRUN_FIFO_DEST 0xC0000Cu
#define STUNRUN_FIFO_TERMINATOR 0xFFFFu

/* Observed PCs: caller, callee, transfer helper, count/terminator
 * readers, and transfer-loop PCs. */
#define STUNRUN_FIFO_CALLER_PC 0x02C5EAu
#define STUNRUN_FIFO_CALLEE_PC 0x02F0AEu
#define STUNRUN_FIFO_HELPER_PC 0x02247Au
#define STUNRUN_FIFO_COUNT_PC 0x02F0C2u
#define STUNRUN_FIFO_TERM_PC 0x02F0E2u
#define STUNRUN_FIFO_LOOP_A_PC 0x02248Eu
#define STUNRUN_FIFO_LOOP_B_PC 0x02249Au

/* 14 callee executions per 600-frame run, both modes. */
#define STUNRUN_FIFO_CALLS_PER_RUN 14u

/* 1200-frame run summary, both modes identical. */
#define STUNRUN_FIFO_RUN_BLOCKS 106u
#define STUNRUN_FIFO_RUN_TERMINATORS 106u
#define STUNRUN_FIFO_RUN_READS 625989u

/* Terminator halfword address for a block starting at base whose header
 * count is count: base + 2*count (tap-observed formula). */
uint32_t stunrun_fifo_terminator_address(uint32_t base, unsigned count);

/* Block validation mirroring the callee's -1 check: nonzero iff the
 * terminator halfword equals 0xFFFF. */
int stunrun_fifo_block_valid(uint16_t terminator);

/* Transfer mirroring 0x02247A: emit exactly count FIFO writes from
 * payload[0..count-1] into fifo_out. Returns the write count, or 0 when
 * any pointer is NULL or fifo_cap < count (no partial writes). A zero
 * count with valid pointers succeeds trivially. */
size_t stunrun_fifo_transfer(const uint16_t *payload, unsigned count,
                             uint16_t *fifo_out, size_t fifo_cap);

/* 1200-frame observed run summary. */
typedef struct stunrun_fifo_run_summary {
    unsigned blocks;
    unsigned terminators;
    unsigned reads;
    int fifo_writes_match_count;
} stunrun_fifo_run_summary_t;

stunrun_fifo_run_summary_t stunrun_fifo_observed_run(void);

/* Nonzero iff every field equals the observed run (NULL never matches). */
int stunrun_fifo_run_matches(const stunrun_fifo_run_summary_t *summary);

#ifdef __cplusplus
}
#endif

#endif /* STUNRUN_FIFO_BLOCK_H */
