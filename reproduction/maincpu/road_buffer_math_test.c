/* ROM-free self-check for the observed byte-wise road-buffer arithmetic. */
#include <stdio.h>

#include "road_buffer_math.h"

static int failures;

#define check(cond) do { \
    if (!(cond)) { printf("FAIL %s\n", #cond); failures++; } \
} while (0)

int main(void)
{
    const uint8_t source[] = {0x00u, 0x01u, 0x7Fu, 0x80u, 0xFFu};
    uint8_t copied_base[sizeof(source)] = {0};
    uint8_t copied_twin[sizeof(source)] = {0};
    uint8_t base[] = {0x00u, 0x10u, 0x80u, 0xFFu};
    const uint8_t twin[] = {0x01u, 0x20u, 0x81u, 0x02u};

    stunrun_road_buffer_subtract(base, twin, sizeof(base));
    check(base[0] == 0xFFu);
    check(base[1] == 0xF0u);
    check(base[2] == 0xFFu);
    check(base[3] == 0xFDu);

    stunrun_road_buffer_add(base, twin, sizeof(base));
    check(base[0] == 0x00u);
    check(base[1] == 0x10u);
    check(base[2] == 0x80u);
    check(base[3] == 0xFFu);

    check(stunrun_road_buffer_copy_scaled(source, copied_base, copied_twin,
                                          2u, sizeof(source)) == 1);
    check(copied_base[0] == 0x00u && copied_base[4] == 0xFFu);
    check(copied_twin[0] == 0x00u && copied_twin[1] == 0x00u);
    check(copied_twin[2] == 0x3Fu && copied_twin[3] == 0x40u);
    check(copied_twin[4] == 0x7Fu);
    check(stunrun_road_buffer_copy_scaled(source, copied_base, copied_twin,
                                          0u, sizeof(source)) == 0);

    check(STUNRUN_ROAD_BUFFER_BYTES == 0x300u);
    stunrun_road_buffer_add(NULL, twin, sizeof(base));
    stunrun_road_buffer_subtract(base, NULL, sizeof(base));
    if (failures == 0) {
        printf("road buffer math C port: all checks passed\n");
    }
    return failures != 0;
}
