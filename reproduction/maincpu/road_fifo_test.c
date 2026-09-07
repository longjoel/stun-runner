/* ROM-free self-check for the observed road-buffer FIFO drain. */
#include <stdio.h>

#include "road_fifo.h"

static int failures;

#define check(cond) do { \
    if (!(cond)) { printf("FAIL %s\n", #cond); failures++; } \
} while (0)

int main(void)
{
    uint16_t base[STUNRUN_ROAD_FIFO_SOURCE_WORDS];
    uint16_t fifo[STUNRUN_ROAD_FIFO_WRITES];
    size_t i;

    for (i = 0; i < STUNRUN_ROAD_FIFO_SOURCE_WORDS; i++) {
        base[i] = (uint16_t)(0x4000u + i);
    }
    for (i = 0; i < STUNRUN_ROAD_FIFO_WRITES; i++) {
        fifo[i] = 0xDEADu;
    }
    check(STUNRUN_ROAD_FIFO_SOURCE_WORDS == 384u);
    check(STUNRUN_ROAD_FIFO_WRITES == 192u);
    check(STUNRUN_ROAD_FIFO_PC == 0x02248Eu);
    check(STUNRUN_ROAD_FIFO_DEST == 0xC0000Cu);
    check(stunrun_road_fifo_drain(base, STUNRUN_ROAD_FIFO_SOURCE_WORDS, fifo,
                                  STUNRUN_ROAD_FIFO_WRITES) ==
          STUNRUN_ROAD_FIFO_WRITES);
    check(fifo[0] == base[0]);
    check(fifo[STUNRUN_ROAD_FIFO_WRITES - 1u] == base[382]);
    check(stunrun_road_fifo_drain(base, STUNRUN_ROAD_FIFO_SOURCE_WORDS - 1u,
                                  fifo, STUNRUN_ROAD_FIFO_WRITES) == 0u);
    check(stunrun_road_fifo_drain(base, STUNRUN_ROAD_FIFO_SOURCE_WORDS, fifo,
                                  1u) ==
          0u);
    check(stunrun_road_fifo_drain(NULL, STUNRUN_ROAD_FIFO_SOURCE_WORDS, fifo,
                                  STUNRUN_ROAD_FIFO_WRITES) == 0u);
    if (failures == 0) {
        printf("road fifo C port: all checks passed\n");
    }
    return failures != 0;
}
