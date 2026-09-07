#include "trajectory_state.h"

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
