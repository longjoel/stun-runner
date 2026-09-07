#include "trajectory_state.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    assert(stunrun_trajectory_initial() == (int16_t)0xFCA0);
    assert(stunrun_trajectory_clamp(0) == (int16_t)0xFCA0);
    assert(stunrun_trajectory_clamp(-0x360) == (int16_t)-0x360);
    assert(stunrun_trajectory_clamp(-0x5A0) == (int16_t)-0x5A0);
    assert(stunrun_trajectory_clamp(-0x5A1) == (int16_t)0xFA60);
    assert(stunrun_trajectory_step((int16_t)0xFCA0, -1) ==
           (int16_t)0xFC9F);
    assert(stunrun_trajectory_step((int16_t)0xFCA0, 1) ==
           (int16_t)0xFCA0);
    assert(stunrun_trajectory_step((int16_t)0xFA60, 1) ==
           (int16_t)0xFA61);
    puts("trajectory-state tests passed");
    return 0;
}
