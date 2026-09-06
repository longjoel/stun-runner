/* ROM-free self-check for the FIFO block-transfer C model.
 *
 * Vectors from adsp-buffer-window.metadata.json (106 blocks, terminator
 * formula, 625989 reads) and adsp-interface-map.metadata.json (14
 * executions, -1 check, one FIFO write per count unit). Exit 0.
 */
#include <stdio.h>

#include "fifo_block.h"

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
    static const uint16_t kPayload[] = { 0xB800u, 0x0105u, 0x0103u,
                                         0x0013u, 0x0004u };
    uint16_t fifo[8];
    uint16_t guarded[4];
    stunrun_fifo_run_summary_t run;
    unsigned i;

    /* Terminator formula: base + 2*count. */
    check(stunrun_fifo_terminator_address(STUNRUN_FIFO_SOURCE_BASE, 1u) ==
          0x810002u);
    check(stunrun_fifo_terminator_address(0x810000u, 0u) == 0x810000u);
    check(stunrun_fifo_terminator_address(STUNRUN_FIFO_SOURCE_BASE,
                                          106u) ==
          STUNRUN_FIFO_SOURCE_BASE + 212u);

    /* The -1 check accepts 0xFFFF only. */
    check(stunrun_fifo_block_valid(STUNRUN_FIFO_TERMINATOR));
    check(stunrun_fifo_block_valid(0x0000u) == 0);
    check(stunrun_fifo_block_valid(0xFFFEu) == 0);

    /* Transfer emits exactly count writes, in order (payload opens with
     * the observed title-path FIFO data prefix). */
    for (i = 0; i < 8u; i++)
        fifo[i] = 0xDEADu;
    check(stunrun_fifo_transfer(kPayload, 5u, fifo, 8u) == 5u);
    check(fifo[0] == 0xB800u && fifo[1] == 0x0105u &&
          fifo[2] == 0x0103u && fifo[3] == 0x0013u &&
          fifo[4] == 0x0004u);
    check(fifo[5] == 0xDEADu);

    /* Zero count succeeds trivially; undersized capacity and NULLs fail
     * with no partial writes. */
    check(stunrun_fifo_transfer(kPayload, 0u, fifo, 8u) == 0u);
    for (i = 0; i < 4u; i++)
        guarded[i] = 0xBEEFu;
    check(stunrun_fifo_transfer(kPayload, 5u, guarded, 4u) == 0u);
    check(guarded[0] == 0xBEEFu && guarded[3] == 0xBEEFu);
    check(stunrun_fifo_transfer(NULL, 5u, fifo, 8u) == 0u);
    check(stunrun_fifo_transfer(kPayload, 5u, NULL, 8u) == 0u);

    /* Run summary: the observed 1200-frame table matches; any single
     * perturbation or NULL does not. */
    run = stunrun_fifo_observed_run();
    check(run.blocks == 106u && run.terminators == 106u);
    check(run.reads == 625989u && run.fifo_writes_match_count == 1);
    check(stunrun_fifo_run_matches(&run));
    run.blocks = 105u;
    check(!stunrun_fifo_run_matches(&run));
    run = stunrun_fifo_observed_run();
    run.fifo_writes_match_count = 0;
    check(!stunrun_fifo_run_matches(&run));
    check(!stunrun_fifo_run_matches(NULL));

    /* Contract constants. */
    check(STUNRUN_FIFO_DEST == 0xC0000Cu);
    check(STUNRUN_FIFO_CALLS_PER_RUN == 14u);
    check(STUNRUN_FIFO_CALLER_PC == 0x02C5EAu);
    check(STUNRUN_FIFO_CALLEE_PC == 0x02F0AEu);
    check(STUNRUN_FIFO_HELPER_PC == 0x02247Au);

    if (g_failures == 0)
        printf("fifo block C port: all checks passed\n");
    return g_failures == 0 ? 0 : 1;
}
