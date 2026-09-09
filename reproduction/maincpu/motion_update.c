#include "motion_update.h"

int16_t stunrun_motion_update(int16_t current, uint16_t timer,
                              int16_t local_limit, int16_t source_a,
                              int16_t source_b)
{
    int32_t value = current;

    if (timer != 0u && value < STUNRUN_MOTION_RAMP_LIMIT)
        value += STUNRUN_MOTION_RAMP_STEP;
    else if (value > (int32_t)local_limit + STUNRUN_MOTION_RAMP_STEP)
        value -= STUNRUN_MOTION_DECAY_STEP;
    else
        value = (int32_t)source_b + source_a;
    return (int16_t)value;
}
