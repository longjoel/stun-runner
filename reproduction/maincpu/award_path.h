/* Object-hit -> award-select -> score-add chain, in C.
 *
 * Agent 2 (Implementer) encoding of the instruction-level award
 * contract. score.c already models the accumulator behavior; this slice
 * freezes the three-PC event chain with its instruction-stream
 * immediates. Literal mechanism only.
 *
 * Provenance (OBSERVED-IN-TRACE, course-5 replay window frames
 * 1400-1520, two award events at frames 1440/1488, identical reads):
 * reference/experiments/stunrun/geometry-residue-capture.metadata.json
 * is not the source; see QUESTIONS.md IRQ-0005 update 8 and the local
 * /tmp/race-award trace (30,240 events, no truncation).
 * - Hit counter PC 0x03A298 reads dest-immediate 0xFFDD02 fragments at
 *   0x3A29A (0xFF) and 0x3A29C (0xDD02); the 0x03A298/0x3A29E words are
 *   opcode fetches (0x5279/0x3039).
 * - Select PC 0x03A2E2 reads 0x3A2E4 = 0x0 (role unresolved) and the
 *   award-high immediate 0x3A2E6 = 0x1F4 (500); 0x3A2E8 = 0x6002 is the
 *   following opcode word.
 * - Add PC 0x03A2EC reads dest-immediate 0xFF9532 fragments at 0x3A2EE
 *   (0xFF) and 0x3A2F0 (0x9532); 0x3A2EC/0x3A2F2 are opcode words.
 * - Behavior: 0x1F4 selected when the 0xFF9578 course word is nonzero
 *   (live at course 5 and 10: clean +500 awards; course-0 sweep run
 *   scored purely in +50 steps):
 *   reference/experiments/stunrun/main-ram-center-object-score.metadata.json
 *   plus race-run-3/race2 course logs (local).
 * - Collision-check caller 0x03B02C is STATIC context from the same
 *   metadata, not traced here.
 *
 * Deliberately NOT claimed: the 50-award immediate address (the
 * zero path never executes under a nonzero course and no course-0
 * ROM-read trace exists); what 0x3A2E4's 0x0 operand means; hit
 * detection itself (upstream of 0x03A298).
 *
 * No ROM contents are embedded or required.
 */
#ifndef STUNRUN_AWARD_PATH_H
#define STUNRUN_AWARD_PATH_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Event PCs. */
#define STUNRUN_AWARD_HIT_PC 0x03A298u
#define STUNRUN_AWARD_SELECT_PC 0x03A2E2u
#define STUNRUN_AWARD_ADD_PC 0x03A2ECu

/* Instruction-stream immediates (addresses hold the stated words). */
#define STUNRUN_AWARD_HIT_DEST_HI_ADDR 0x3A29Au
#define STUNRUN_AWARD_HIT_DEST_LO_ADDR 0x3A29Cu
#define STUNRUN_AWARD_SELECT_IMM_ADDR 0x3A2E6u
#define STUNRUN_AWARD_ADD_DEST_HI_ADDR 0x3A2EEu
#define STUNRUN_AWARD_ADD_DEST_LO_ADDR 0x3A2F0u

/* Frozen immediate values. */
#define STUNRUN_AWARD_HIT_DEST_WORD 0xFFDD02u
#define STUNRUN_AWARD_HIGH_VALUE 0x1F4u
#define STUNRUN_AWARD_LOW_VALUE 0x32u
#define STUNRUN_AWARD_SCORE_WORD 0xFF9532u

/* One hit event through the three-PC chain. A miss produces all zeros.
 * `course_value` is the raw 0xFF9578 word (nonzero selects 0x1F4). */
typedef struct stunrun_award_event {
    unsigned counter_inc;
    uint16_t award;
    uint16_t score_add;
} stunrun_award_event_t;

stunrun_award_event_t stunrun_award_event(unsigned course_value, int hit);

#ifdef __cplusplus
}
#endif

#endif /* STUNRUN_AWARD_PATH_H */
