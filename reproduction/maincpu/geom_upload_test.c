/* ROM-free self-check for the geometry-upload C model.
 *
 * Synthetic 384-word table only; no ROM contents. Golden vectors are the
 * frozen mechanism constants (march shape, copy count, table bases,
 * twin stride) from geom_upload.h provenance.
 */
#include <stdio.h>

#include "geom_upload.h"

static int g_failures = 0;

#define check(cond) \
    do { \
        if (cond) { \
            printf("PASS %s\n", #cond); \
        } else { \
            printf("FAIL %s\n", #cond); \
            g_failures++; \
        } \
    } while (0)

int main(void)
{
    static uint16_t table[STUNRUN_GEOM_MARCH_WORDS];
    static uint16_t image[STUNRUN_GEOM_MARCH_WORDS];
    static uint16_t dst_a[STUNRUN_GEOM_MARCH_WORDS];
    static uint16_t dst_b[STUNRUN_GEOM_MARCH_WORDS];
    unsigned i;
    uint32_t base = 0;

    for (i = 0; i < STUNRUN_GEOM_MARCH_WORDS; i++) {
        table[i] = (uint16_t)(0xA500u + i);
        image[i] = 0;
        dst_a[i] = 0;
        dst_b[i] = 0;
    }

    /* Frozen shape constants. */
    check(STUNRUN_GEOM_MARCH_WORDS == 384u);
    check(STUNRUN_GEOM_MARCH_STEP == 2u);
    check(STUNRUN_GEOM_COPY_COUNT == 2u);
    check(STUNRUN_GEOM_TWIN_STRIDE == 0x300u);

    /* Frozen table bases share the 0x230 (mod 0x300) slot congruence:
     * mutually 0x300-spaced (0x300, 0xC00, 0x300 apart). */
    check(STUNRUN_GEOM_TABLE_C0 % STUNRUN_GEOM_TWIN_STRIDE == 0x230u);
    check(STUNRUN_GEOM_TABLE_C5 % STUNRUN_GEOM_TWIN_STRIDE == 0x230u);
    check(STUNRUN_GEOM_TABLE_C10 % STUNRUN_GEOM_TWIN_STRIDE == 0x230u);
    check(STUNRUN_GEOM_TABLE_C11_C12 % STUNRUN_GEOM_TWIN_STRIDE == 0x230u);
    check(STUNRUN_GEOM_TABLE_C5 - STUNRUN_GEOM_TABLE_C0 == 0xC00u);
    check(stunrun_geom_observed_table_base(0u, &base) &&
          base == STUNRUN_GEOM_TABLE_C0);
    check(stunrun_geom_observed_table_base(5u, &base) &&
          base == STUNRUN_GEOM_TABLE_C5);
    check(stunrun_geom_observed_table_base(10u, &base) &&
          base == STUNRUN_GEOM_TABLE_C10);
    check(stunrun_geom_observed_table_base(11u, &base) &&
          base == STUNRUN_GEOM_TABLE_C11_C12);
    check(stunrun_geom_observed_table_base(12u, &base) &&
          base == STUNRUN_GEOM_TABLE_C11_C12);
    check(!stunrun_geom_observed_table_base(1u, &base));
    check(!stunrun_geom_observed_table_base(0u, NULL));

    /* March preserves ascending order over all 384 words. */
    stunrun_geom_march(table, image);
    for (i = 0; i < STUNRUN_GEOM_MARCH_WORDS; i++) {
        if (image[i] != table[i]) {
            break;
        }
    }
    check(i == STUNRUN_GEOM_MARCH_WORDS);

    /* Fan-out writes identical copies to both destinations. */
    stunrun_geom_fanout(image, dst_a, dst_b);
    for (i = 0; i < STUNRUN_GEOM_MARCH_WORDS; i++) {
        if (dst_a[i] != table[i] || dst_b[i] != table[i]) {
            break;
        }
    }
    check(i == STUNRUN_GEOM_MARCH_WORDS);

    /* Full upload gates on the header count. */
    for (i = 0; i < STUNRUN_GEOM_MARCH_WORDS; i++) {
        dst_a[i] = 0;
        dst_b[i] = 0;
    }
    check(stunrun_geom_upload(STUNRUN_GEOM_COPY_COUNT, table, dst_a,
                              dst_b) == 2u);
    check(dst_a[0] == table[0]);
    check(dst_a[STUNRUN_GEOM_MARCH_WORDS - 1u] ==
          table[STUNRUN_GEOM_MARCH_WORDS - 1u]);
    check(dst_b[0] == table[0]);
    check(dst_b[STUNRUN_GEOM_MARCH_WORDS - 1u] ==
          table[STUNRUN_GEOM_MARCH_WORDS - 1u]);
    check(stunrun_geom_upload(0u, table, dst_a, dst_b) == 0u);
    check(stunrun_geom_upload(1u, table, dst_a, dst_b) == 0u);
    check(stunrun_geom_upload(3u, table, dst_a, dst_b) == 0u);

    /* Frozen destination mapping: twin sits exactly one stride past base,
     * inside the observed residue twin families. */
    check(STUNRUN_GEOM_DEST_TWIN - STUNRUN_GEOM_DEST_BASE ==
          STUNRUN_GEOM_TWIN_STRIDE);
    check(STUNRUN_GEOM_DEST_BASE == 0xFF9584u);
    check(STUNRUN_GEOM_DEST_TWIN == 0xFF9884u);

    /* NULL inputs are safe no-ops. */
    stunrun_geom_march(NULL, image);
    stunrun_geom_march(table, NULL);
    stunrun_geom_fanout(NULL, dst_a, dst_b);
    stunrun_geom_fanout(image, NULL, NULL);
    check(stunrun_geom_upload(STUNRUN_GEOM_COPY_COUNT, NULL, dst_a,
                              dst_b) == 0u);
    check(stunrun_geom_upload(STUNRUN_GEOM_COPY_COUNT, table, NULL,
                              dst_b) == 0u);

    if (g_failures == 0) {
        printf("GEOM_UPLOAD_ALL_PASS\n");
    }
    return g_failures != 0;
}
