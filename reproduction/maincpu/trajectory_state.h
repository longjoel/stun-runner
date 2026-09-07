/* Literal 68010 trajectory-state slice.
 *
 * Provenance: generated main-CPU listing around 0x0387C6-0x038A14 and the
 * state-600 Button 2 early-tap fork in
 * reference/experiments/stunrun/state-600-button2-early-tap.metadata.json.
 *
 * The listing establishes initialization of 0xFFDCC6 to 0xFCA0, an update
 * driven by the motion delta, and the two observed signed bounds. It does
 * not establish the physical axis or the identity of the effect/object.
 */
#ifndef STUNRUN_TRAJECTORY_STATE_H
#define STUNRUN_TRAJECTORY_STATE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STUNRUN_TRAJECTORY_INITIAL  ((int16_t)-0x360)
#define STUNRUN_TRAJECTORY_RESET    ((int16_t)-0x360)
#define STUNRUN_TRAJECTORY_LOWER    ((int16_t)-0x5A0)

/* The observed 68010 literal is 0xFCA0, which is -0x360. */
int16_t stunrun_trajectory_initial(void);

/* Apply the literal post-add clamp from 0x038942-0x038966. */
int16_t stunrun_trajectory_clamp(int32_t value);

/* Add a signed integrator delta and apply that clamp. */
int16_t stunrun_trajectory_step(int16_t coordinate, int16_t delta);

#ifdef __cplusplus
}
#endif

#endif /* STUNRUN_TRAJECTORY_STATE_H */
