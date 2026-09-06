/* See score.h for provenance and confidence notes. */
#include "score.h"

uint16_t stunrun_score_select_award(unsigned course_value)
{
    return course_value != 0 ? STUNRUN_SCORE_AWARD_HIGH
                             : STUNRUN_SCORE_AWARD_LOW;
}

void stunrun_score_init(stunrun_score_accum_t *acc)
{
    if (acc == NULL)
        return;
    acc->total = 0;
}

void stunrun_score_clear(stunrun_score_accum_t *acc)
{
    stunrun_score_init(acc);
}

void stunrun_score_add(stunrun_score_accum_t *acc, uint16_t award)
{
    if (acc == NULL)
        return;
    acc->total += award;
}

void stunrun_hit_counter_init(stunrun_hit_counter_t *hc)
{
    if (hc == NULL)
        return;
    hc->count = 0;
}

void stunrun_hit_counter_hit(stunrun_hit_counter_t *hc)
{
    if (hc == NULL)
        return;
    hc->count++;
}
