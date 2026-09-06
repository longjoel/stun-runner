/* ROM-free self-check for the score/hit-counter C model.
 *
 * Golden vectors transcribed from main-ram-score-candidate.metadata.json
 * (weapon_probe run: ten +50 writes, cumulative 50..500 at 0xFF9534) and
 * main-ram-center-object-score.metadata.json (award selection 50/500,
 * hit-counter values 0x01..0x22). Exit 0 on success.
 */
#include <stdio.h>

#include "score.h"

static int g_failures = 0;

#define check(cond) \
    do { \
        if (cond) { \
            printf("PASS %s\n", #cond); \
        } else { \
            printf("FAIL %s\n", #cond); \
            g_failures++; \
        } \
    } while (0)

int main(void)
{
    stunrun_score_accum_t acc;
    stunrun_hit_counter_t hc;
    unsigned i;

    /* Award selection: 0x1F4 when the course value is nonzero (observed
     * values 3 and 6), else 0x32. */
    check(stunrun_score_select_award(STUNRUN_SCORE_COURSE_ZERO) == 50u);
    check(stunrun_score_select_award(STUNRUN_SCORE_COURSE_A) == 500u);
    check(stunrun_score_select_award(STUNRUN_SCORE_COURSE_B) == 500u);
    check(STUNRUN_SCORE_AWARD_LOW == 0x32u);
    check(STUNRUN_SCORE_AWARD_HIGH == 0x1F4u);

    /* weapon_probe golden run: ten +50 awards accumulate 50..500 in the
     * low word while the high word stays zero. */
    stunrun_score_init(&acc);
    check(acc.total == 0u);
    for (i = 0; i < STUNRUN_SCORE_GOLDEN_AWARDS; i++) {
        stunrun_score_add(&acc,
                          stunrun_score_select_award(STUNRUN_SCORE_COURSE_ZERO));
        check(stunrun_score_low_word(&acc) == 50u * (i + 1u));
        check(stunrun_score_high_word(&acc) == 0u);
    }
    check(acc.total == 500u);

    /* A high award lands on top of existing score through the same add. */
    stunrun_score_add(&acc,
                      stunrun_score_select_award(STUNRUN_SCORE_COURSE_A));
    check(acc.total == 1000u);

    /* Clear returns to zero (the four observed clear sites). */
    stunrun_score_clear(&acc);
    check(acc.total == 0u);

    /* NULL accumulators are safe no-ops. */
    stunrun_score_init(NULL);
    stunrun_score_clear(NULL);
    stunrun_score_add(NULL, 50u);

    /* Hit counter: the center run observed values 0x01..0x22, i.e. 34
     * increments from zero. */
    stunrun_hit_counter_init(&hc);
    check(hc.count == 0u);
    for (i = 0; i < 0x22u; i++)
        stunrun_hit_counter_hit(&hc);
    check(hc.count == 0x22u);
    stunrun_hit_counter_init(NULL);
    stunrun_hit_counter_hit(NULL);

    if (g_failures == 0)
        printf("score C port: all checks passed\n");
    return g_failures == 0 ? 0 : 1;
}
