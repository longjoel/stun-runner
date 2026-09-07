#include "trajectory_state.h"

#include <stddef.h>

int16_t stunrun_trajectory_initial(void)
{
    return STUNRUN_TRAJECTORY_INITIAL;
}

int16_t stunrun_trajectory_clamp(int32_t value)
{
    /* Literal branch order:
     *   if value > -0x360: value = -0x360
     *   else if value < -0x5a0: value = -0x5a0
     */
    if (value > STUNRUN_TRAJECTORY_RESET)
        return STUNRUN_TRAJECTORY_RESET;
    if (value < STUNRUN_TRAJECTORY_LOWER)
        return STUNRUN_TRAJECTORY_LOWER;
    return (int16_t)value;
}

int16_t stunrun_trajectory_step(int16_t coordinate, int16_t delta)
{
    return stunrun_trajectory_clamp((int32_t)coordinate + delta);
}

static int32_t arithmetic_shift_right_one(int32_t value)
{
    if (value >= 0)
        return value / 2;
    /* Define the 68010 sign-preserving shift for negative odd values. */
    return -((-value + 1) / 2);
}

int16_t stunrun_trajectory_filter_delta(int16_t d2_delta, int16_t d0_delta,
                                        int16_t *stored_dcd2)
{
    int32_t d3 = (int32_t)d2_delta - d0_delta;
    int32_t result;

    if (d3 != 0) {
        result = arithmetic_shift_right_one(d3);
        if (result == 0)
            result = -1;
        else
            result = -result;
    } else {
        result = stored_dcd2 == NULL ? 0 : *stored_dcd2;
        if (result < 0)
            result = -result;
    }
    if (stored_dcd2 != NULL)
        *stored_dcd2 = (int16_t)result;
    return (int16_t)result;
}
