/* Course-table march and twin-buffer fan-out. See geom_upload.h. */

#include "geom_upload.h"

void stunrun_geom_march(const uint16_t *table, uint16_t *image)
{
    size_t i;

    if (table == NULL || image == NULL) {
        return;
    }
    for (i = 0; i < STUNRUN_GEOM_MARCH_WORDS; i++) {
        image[i] = table[i];
    }
}

void stunrun_geom_fanout(const uint16_t *image, uint16_t *dst_a,
                         uint16_t *dst_b)
{
    size_t i;

    if (image == NULL) {
        return;
    }
    for (i = 0; i < STUNRUN_GEOM_MARCH_WORDS; i++) {
        if (dst_a != NULL) {
            dst_a[i] = image[i];
        }
        if (dst_b != NULL) {
            dst_b[i] = image[i];
        }
    }
}

unsigned stunrun_geom_upload(unsigned count, const uint16_t *table,
                             uint16_t *dst_a, uint16_t *dst_b)
{
    uint16_t image[STUNRUN_GEOM_MARCH_WORDS];
    size_t i;

    if (count != STUNRUN_GEOM_COPY_COUNT) {
        return 0;
    }
    if (table == NULL || dst_a == NULL || dst_b == NULL) {
        return 0;
    }
    /* Pass A (PC 0x29760) and pass B (PC 0x2976E) march the same table;
     * per-copy writer loops then store each pass to its own buffer. */
    stunrun_geom_march(table, image);
    stunrun_geom_march(table, image);
    for (i = 0; i < STUNRUN_GEOM_MARCH_WORDS; i++) {
        dst_a[i] = image[i];
    }
    for (i = 0; i < STUNRUN_GEOM_MARCH_WORDS; i++) {
        dst_b[i] = image[i];
    }
    return 2;
}
