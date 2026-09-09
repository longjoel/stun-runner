#include "motion_update.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    assert(stunrun_motion_update(0x0100, 1u, 0x07E0, 0x0011, 0x0022) ==
           (int16_t)0x0140);
    assert(stunrun_motion_update(0x0B00, 1u, 0x07E0, 0, 0) ==
           (int16_t)0x0AE0);
    assert(stunrun_motion_update(0x0900, 0u, 0x07E0, 0, 0) ==
           (int16_t)0x08E0);
    assert(stunrun_motion_update(0x0820, 0u, 0x07E0, 0x0011, 0x0022) ==
           (int16_t)0x0033);
    puts("motion-update tests passed");
    return 0;
}
