/* 68010 live-score accumulator and object-hit counter, in C.
 *
 * Agent 2 (Implementer) encoding of Agent 1's frozen score findings.
 * Literal mechanism only.
 *
 * Provenance:
 * - Accumulator identity (0xFF9532-0xFF9535 longword), clear sites,
 *   display reads, and the weapon_probe golden run (ten monotonic +50
 *   writes at 0xFF9534 from PC 0x03A2EC, values 50..500, high word zero):
 *   reference/experiments/stunrun/main-ram-score-candidate.metadata.json
 *   (DYNAMIC-SCORE-CONFIRMED).
 * - Award selection (0x03A2E2 selects 0x1F4/500 when 0xFF9578 nonzero,
 *   else 0x32/50; 0x03A2EC adds the selection) and hit-counter range
 *   (0xFFDD02 incremented at 0x03A298, values 0x01..0x22):
 *   reference/experiments/stunrun/main-ram-center-object-score.
 *   metadata.json (OBJECT_HIT_AND_SCORE_CONFIRMED).
 * - Observed course-index values 0/3/6: analysis/ram-map.md.
 *
 * Deliberately NOT claimed: award values at the other static addition
 * sites (0x028DFA, 0x028E54, 0x028F1C, timer path) are not modeled;
 * counter width and wrap behavior are unobserved; live-score-to-high-
 * score promotion is open. The course-index semantic (track numbering)
 * is not assigned — selection takes a nonzero flag, not a track.
 *
 * No ROM contents are embedded or required.
 */
#ifndef STUNRUN_SCORE_H
#define STUNRUN_SCORE_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Conceptual addresses (68010 view) and writer PCs. */
#define STUNRUN_SCORE_ADDR 0xFF9532u
#define STUNRUN_SCORE_SELECT_PC 0x03A2E2u
#define STUNRUN_SCORE_AWARD_PC 0x03A2ECu
#define STUNRUN_SCORE_HIT_COUNTER_ADDR 0xFFDD02u
#define STUNRUN_SCORE_HIT_PC 0x03A298u
#define STUNRUN_SCORE_COURSE_ADDR 0xFF9578u

/* Observed award values (0x03A2E2 selection). */
#define STUNRUN_SCORE_AWARD_LOW 0x32u
#define STUNRUN_SCORE_AWARD_HIGH 0x1F4u

/* Course-index values for the selection path. Live provenance updated
 * 2026-09-06: human races observed 0xFF9579 = 5 (frame 880, writer PC
 * 0x02B63E), 10 and 11, with +500 awards firing at 5 and 10, so the
 * nonzero=>500 selection is OBSERVED-IN-TRACE. Values 3/6 below remain
 * static-only regression vectors for the same nonzero path. */
#define STUNRUN_SCORE_COURSE_ZERO 0u
#define STUNRUN_SCORE_COURSE_A 3u
#define STUNRUN_SCORE_COURSE_B 6u

/* weapon_probe golden run: ten awards, cumulative low-word values. */
#define STUNRUN_SCORE_GOLDEN_AWARDS 10u

/* Mirrors 0x03A2E2: 500 when the course value is nonzero, else 50. */
uint16_t stunrun_score_select_award(unsigned course_value);

/* Live accumulator: the 0xFF9532 longword. Observed in the low-word
 * range (high word zero); width and wrap are not established. */
typedef struct stunrun_score_accum {
    uint32_t total;
} stunrun_score_accum_t;

void stunrun_score_init(stunrun_score_accum_t *acc);
void stunrun_score_clear(stunrun_score_accum_t *acc);

/* Mirrors 0x03A2EC: add the selected award to the accumulator. */
void stunrun_score_add(stunrun_score_accum_t *acc, uint16_t award);

static inline uint16_t stunrun_score_low_word(
    const stunrun_score_accum_t *acc)
{
    return (uint16_t)(acc->total & 0xFFFFu);
}

static inline uint16_t stunrun_score_high_word(
    const stunrun_score_accum_t *acc)
{
    return (uint16_t)(acc->total >> 16);
}

/* Object-hit counter: the 0xFFDD02 progression count. */
typedef struct stunrun_hit_counter {
    unsigned count;
} stunrun_hit_counter_t;

void stunrun_hit_counter_init(stunrun_hit_counter_t *hc);

/* Mirrors 0x03A298: add one after a successful collision check. */
void stunrun_hit_counter_hit(stunrun_hit_counter_t *hc);

#ifdef __cplusplus
}
#endif

#endif /* STUNRUN_SCORE_H */
