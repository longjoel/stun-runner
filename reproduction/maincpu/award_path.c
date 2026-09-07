/* Object-hit -> award-select -> score-add chain. See award_path.h. */

#include "award_path.h"

stunrun_award_event_t stunrun_award_event(unsigned course_value, int hit)
{
    stunrun_award_event_t event = {0, 0, 0};

    if (!hit) {
        return event;
    }
    /* PC 0x03A298: increment the 0xFFDD02 hit counter. */
    event.counter_inc = 1;
    /* PC 0x03A2E2: select 0x1F4 when the course word is nonzero,
     * else 0x32. */
    event.award = course_value != 0 ? STUNRUN_AWARD_HIGH_VALUE
                                    : STUNRUN_AWARD_LOW_VALUE;
    /* PC 0x03A2EC: add the selected award to 0xFF9532. */
    event.score_add = event.award;
    return event;
}
