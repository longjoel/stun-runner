#include "fake_ports.h"
#include "game_loop.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
    stunrun_fake_ports_t ports;
    stunrun_game_loop_t loop;
    uint32_t tick = 0;
    uint16_t motion_delta = 0;
    int32_t steering = 0;
    int32_t button = 0;
    uint16_t table[STUNRUN_GAME_LOOP_ROAD_WORDS];
    size_t i;

    stunrun_ports_init(&ports);
    stunrun_game_loop_init(&loop, &ports);
    assert(stunrun_port_set_named(&ports, ":mainpcb:8BADC.0",
                                  "AD Stick X", 220));
    stunrun_game_loop_step(&loop);
    assert(loop.frame == 1u);
    assert(loop.global_tick == 4u);
    assert(loop.steering_delta == 92);
    assert(loop.trajectory_coordinate == (int16_t)-0x360);
    assert(stunrun_mem_read32_be(&ports, 0xFF8014u, &tick) && tick == 4u);
    stunrun_game_loop_set_motion_delta(&loop, -1);
    assert(stunrun_mem_read16_be(&ports, 0xFFDD16u, &motion_delta) &&
           (int16_t)motion_delta == -1);
    stunrun_game_loop_step(&loop);
    assert(loop.trajectory_coordinate == (int16_t)0xFC9F);
    assert(stunrun_port_read(&ports, STUNRUN_PORT_AD_STICK_X, &steering) &&
           steering == 220);
    assert(stunrun_port_set_named(&ports, ":mainpcb:a80000",
                                  "P1 Button 1", 1));
    assert(stunrun_port_get_named(&ports, ":mainpcb:a80000",
                                  "P1 Button 1", &button) && button == 1);
    for (i = 0u; i < STUNRUN_GAME_LOOP_ROAD_WORDS; i++)
        table[i] = (uint16_t)(0x1000u + i);
    assert(stunrun_game_loop_load_road_table(&loop, table));
    assert(stunrun_game_loop_submit_road(&loop) ==
           STUNRUN_GAME_LOOP_FIFO_WORDS);
    assert(ports.gsp_fifo_count == STUNRUN_GAME_LOOP_FIFO_WORDS);
    assert(ports.gsp_fifo[0] == table[0]);
    assert(ports.gsp_fifo[STUNRUN_GAME_LOOP_FIFO_WORDS - 1u] == table[382]);
    puts("native game loop and fake ports: all checks passed");
    return 0;
}
