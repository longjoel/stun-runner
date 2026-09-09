/* Deterministic native game-loop scaffold over the fake machine ports. */
#ifndef STUNRUN_NATIVE_GAME_LOOP_H
#define STUNRUN_NATIVE_GAME_LOOP_H

#include <stddef.h>
#include <stdint.h>

#include "fake_ports.h"

#define STUNRUN_INTERRUPT_TICKS_PER_FRAME 4u
#define STUNRUN_GAME_LOOP_ROAD_WORDS 384u
#define STUNRUN_GAME_LOOP_FIFO_WORDS 192u

typedef struct stunrun_game_loop {
    stunrun_fake_ports_t *ports;
    unsigned frame;
    uint32_t global_tick;
    int32_t steering_delta;
    int16_t trajectory_coordinate;
    int16_t motion_delta;
    uint16_t road_base[STUNRUN_GAME_LOOP_ROAD_WORDS];
    uint16_t road_twin[STUNRUN_GAME_LOOP_ROAD_WORDS];
    uint16_t road_fifo[STUNRUN_GAME_LOOP_FIFO_WORDS];
    size_t road_fifo_count;
    int road_ready;
} stunrun_game_loop_t;

void stunrun_game_loop_init(stunrun_game_loop_t *loop,
                            stunrun_fake_ports_t *ports);
void stunrun_game_loop_step(stunrun_game_loop_t *loop);
void stunrun_game_loop_set_motion_delta(stunrun_game_loop_t *loop,
                                        int16_t motion_delta);
int stunrun_game_loop_load_road_table(stunrun_game_loop_t *loop,
                                      const uint16_t *table);
size_t stunrun_game_loop_submit_road(stunrun_game_loop_t *loop);

#endif
