/* ROM-free self-check for the ADSP control/IRQ/boundary C contract.
 *
 * Every expectation mirrors a checked-in value from
 * reference/experiments/stunrun/adsp-control-window.metadata.json or
 * m3-adsp-upload-boundary.metadata.json. Exit 0 on success, 1 on mismatch.
 */
#include <stdio.h>

#include "adsp_control_seq.h"

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

/* Feed the exact title-path multiset: every op its expected count, all
 * data zero. Mirrors the two identical 600-frame runs. */
static void feed_title_contract(stunrun_adsp_control_tally_t *tally)
{
    size_t i;
    unsigned k;
    const stunrun_adsp_control_op_t *ops = stunrun_adsp_control_ops();
    for (i = 0; i < stunrun_adsp_control_op_count(); i++) {
        for (k = 0; k < ops[i].expected_writes; k++)
            stunrun_adsp_control_tally_add(tally, ops[i].address,
                                           STUNRUN_ADSP_CONTROL_DATA);
    }
}

int main(void)
{
    const stunrun_adsp_control_op_t *ops = stunrun_adsp_control_ops();
    stunrun_adsp_control_tally_t tally;
    unsigned total = 0;
    size_t i;

    check(stunrun_adsp_control_op_count() == 11u);

    /* Table totals reproduce the 73 observed control writes. */
    for (i = 0; i < stunrun_adsp_control_op_count(); i++)
        total += ops[i].expected_writes;
    check(total == STUNRUN_ADSP_CONTROL_TOTAL_WRITES);

    /* Spot checks on the reset/BR/HALT release pairing. */
    check(stunrun_adsp_control_lookup(0x818002u) == 0);
    check(stunrun_adsp_control_lookup(0x81800Eu) == 4);
    check(stunrun_adsp_control_lookup(0x81801Eu) == 10);
    check(stunrun_adsp_control_lookup(0x818004u) == -1);
    check(stunrun_adsp_control_lookup(0x818060u) == -1);

    /* Exact title-path multiset matches. */
    stunrun_adsp_control_tally_init(&tally);
    feed_title_contract(&tally);
    check(stunrun_adsp_control_tally_matches(&tally));

    /* One extra LED write breaks the match. */
    stunrun_adsp_control_tally_init(&tally);
    feed_title_contract(&tally);
    stunrun_adsp_control_tally_add(&tally, 0x818002u,
                                   STUNRUN_ADSP_CONTROL_DATA);
    check(!stunrun_adsp_control_tally_matches(&tally));

    /* An unknown address breaks the match and is counted. */
    stunrun_adsp_control_tally_init(&tally);
    feed_title_contract(&tally);
    check(stunrun_adsp_control_tally_add(&tally, 0x818004u, 0u) == -1);
    check(tally.unknown_addresses == 1u);
    check(!stunrun_adsp_control_tally_matches(&tally));

    /* Nonzero data breaks the match and is counted. */
    stunrun_adsp_control_tally_init(&tally);
    feed_title_contract(&tally);
    stunrun_adsp_control_tally_add(&tally, 0x81800Eu, 0x0001u);
    check(tally.data_mismatches == 1u);
    check(!stunrun_adsp_control_tally_matches(&tally));

    /* Empty tally does not match; NULL tally is safe. */
    stunrun_adsp_control_tally_init(&tally);
    check(!stunrun_adsp_control_tally_matches(&tally));
    check(!stunrun_adsp_control_tally_matches(NULL));
    stunrun_adsp_control_tally_init(NULL);
    check(stunrun_adsp_control_tally_add(NULL, 0x818002u, 0u) == -1);

    /* IRQ-clear pair contract. */
    check(stunrun_adsp_irq_clear_pcs()[0] == 0x021426u);
    check(stunrun_adsp_irq_clear_pcs()[1] == 0x02C234u);
    check(stunrun_adsp_irq_clear_pc_known(0x021426u));
    check(stunrun_adsp_irq_clear_pc_known(0x02C234u));
    check(!stunrun_adsp_irq_clear_pc_known(0x02C676u));

    /* Upload/install frame boundary. */
    check(stunrun_adsp_upload_state_at(0u) == STUNRUN_ADSP_UPLOAD_EMPTY);
    check(stunrun_adsp_upload_state_at(407u) == STUNRUN_ADSP_UPLOAD_EMPTY);
    check(stunrun_adsp_upload_state_at(408u) ==
          STUNRUN_ADSP_UPLOAD_POPULATING);
    check(stunrun_adsp_upload_state_at(410u) ==
          STUNRUN_ADSP_UPLOAD_POPULATING);
    check(stunrun_adsp_upload_state_at(411u) == STUNRUN_ADSP_UPLOAD_COMPLETE);
    check(stunrun_adsp_upload_state_at(600u) == STUNRUN_ADSP_UPLOAD_COMPLETE);
    check(!stunrun_adsp_install_frame_ready(411u));
    check(stunrun_adsp_install_frame_ready(412u));
    check(stunrun_adsp_install_frame_ready(600u));

    if (g_failures == 0)
        printf("adsp control-slice C port: all checks passed\n");
    return g_failures == 0 ? 0 : 1;
}
