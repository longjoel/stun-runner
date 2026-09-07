/* ROM-free self-check for the award-path C model.
 *
 * Synthetic course/hit inputs only. Golden vectors are the frozen
 * immediates (0x1F4 at 0x3A2E6, dest words) and the live award
 * behavior (nonzero course -> +500, zero course -> +50).
 */
#include <stdio.h>

#include "award_path.h"

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
    stunrun_award_event_t event;

    /* Frozen immediates and addresses. */
    check(STUNRUN_AWARD_SELECT_IMM_ADDR == 0x3A2E6u);
    check(STUNRUN_AWARD_HIGH_VALUE == 0x1F4u);
    check(STUNRUN_AWARD_LOW_VALUE == 0x32u);
    check(STUNRUN_AWARD_HIT_DEST_WORD == 0xFFDD02u);
    check(STUNRUN_AWARD_SCORE_WORD == 0xFF9532u);
    check(STUNRUN_AWARD_HIT_PC == 0x03A298u);
    check(STUNRUN_AWARD_SELECT_PC == 0x03A2E2u);
    check(STUNRUN_AWARD_ADD_PC == 0x03A2ECu);

    /* Miss: the chain does nothing. */
    event = stunrun_award_event(5u, 0);
    check(event.counter_inc == 0u);
    check(event.award == 0u);
    check(event.score_add == 0u);

    /* Hit at nonzero course (live: 5 and 10): counter, 500, add 500. */
    event = stunrun_award_event(5u, 1);
    check(event.counter_inc == 1u);
    check(event.award == 500u);
    check(event.score_add == 500u);
    event = stunrun_award_event(10u, 1);
    check(event.counter_inc == 1u);
    check(event.award == 500u);
    check(event.score_add == 500u);

    /* Hit at zero course (live sweep run: +50 steps only). */
    event = stunrun_award_event(0u, 1);
    check(event.counter_inc == 1u);
    check(event.award == 50u);
    check(event.score_add == 50u);

    if (g_failures == 0) {
        printf("AWARD_PATH_ALL_PASS\n");
    }
    return g_failures != 0;
}
