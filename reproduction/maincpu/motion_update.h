/* Literal 68010 update at 0x03ABDE. */
#ifndef STUNRUN_MOTION_UPDATE_H
#define STUNRUN_MOTION_UPDATE_H

#include <stdint.h>

#define STUNRUN_MOTION_RAMP_LIMIT 0x0B00
#define STUNRUN_MOTION_RAMP_STEP  0x0040
#define STUNRUN_MOTION_DECAY_STEP 0x0020

/* Apply the observed branch order to raw 16-bit fields. The caller's timer,
 * local limit, and source words have not been assigned gameplay semantics. */
int16_t stunrun_motion_update(int16_t current, uint16_t timer,
                              int16_t local_limit, int16_t source_a,
                              int16_t source_b);

#endif /* STUNRUN_MOTION_UPDATE_H */
