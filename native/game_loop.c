/* Evidence-shaped native loop. Unresolved fields remain named by mechanism. */
#include "game_loop.h"

#include "trajectory_state.h"
#include "geom_upload.h"
#include "road_fifo.h"

static void service_interrupt_tick(stunrun_game_loop_t *loop)
{
    loop->global_tick++;
    (void)stunrun_mem_write32_be(loop->ports, 0xFF8014u,
                                 loop->global_tick);
}

static void sample_inputs(stunrun_game_loop_t *loop)
{
    int32_t stick_x = 128;
    (void)stunrun_port_read(loop->ports, STUNRUN_PORT_AD_STICK_X, &stick_x);
    loop->steering_delta = stick_x - 128;
}

static void update_trajectory_state(stunrun_game_loop_t *loop)
{
    loop->trajectory_coordinate = stunrun_trajectory_step(
        loop->trajectory_coordinate, loop->motion_delta);
    /* The field is a literal 16-bit big-endian trajectory boundary. Its
     * physical axis and producer are intentionally not named here. */
    (void)stunrun_mem_write16_be(loop->ports, 0xFFDCC6u,
                                 (uint16_t)loop->trajectory_coordinate);
}

void stunrun_game_loop_init(stunrun_game_loop_t *loop,
                            stunrun_fake_ports_t *ports)
{
    if (loop == NULL)
        return;
    loop->ports = ports;
    loop->frame = 0u;
    loop->global_tick = 0u;
    loop->steering_delta = 0;
    loop->trajectory_coordinate = stunrun_trajectory_initial();
    loop->motion_delta = 0;
    loop->road_fifo_count = 0u;
    loop->road_ready = 0;
}

void stunrun_game_loop_step(stunrun_game_loop_t *loop)
{
    unsigned tick;
    if (loop == NULL || loop->ports == NULL)
        return;
    for (tick = 0u; tick < STUNRUN_INTERRUPT_TICKS_PER_FRAME; tick++)
        service_interrupt_tick(loop);
    sample_inputs(loop);
    update_trajectory_state(loop);
    loop->frame++;
}

void stunrun_game_loop_set_motion_delta(stunrun_game_loop_t *loop,
                                        int16_t motion_delta)
{
    if (loop != NULL)
        loop->motion_delta = motion_delta;
}

int stunrun_game_loop_load_road_table(stunrun_game_loop_t *loop,
                                      const uint16_t *table)
{
    if (loop == NULL || table == NULL ||
        stunrun_geom_upload(STUNRUN_GEOM_COPY_COUNT, table,
                            loop->road_base, loop->road_twin) !=
            STUNRUN_GEOM_COPY_COUNT)
        return 0;
    loop->road_fifo_count = 0u;
    loop->road_ready = 1;
    return 1;
}

size_t stunrun_game_loop_submit_road(stunrun_game_loop_t *loop)
{
    size_t i;
    if (loop == NULL || loop->ports == NULL || !loop->road_ready)
        return 0u;
    loop->road_fifo_count = stunrun_road_fifo_drain(
        loop->road_base, STUNRUN_GAME_LOOP_ROAD_WORDS,
        loop->road_fifo, STUNRUN_GAME_LOOP_FIFO_WORDS);
    for (i = 0u; i < loop->road_fifo_count; i++)
        if (!stunrun_gsp_fifo_push(loop->ports, loop->road_fifo[i]))
            return i;
    return loop->road_fifo_count;
}
