/* ROM-free self-check for the ADSP init-slice C port.
 *
 * Each expectation mirrors a checked-in vector from
 * tests/test_replacement_image.py or analysis/ram-map.md, so a failure
 * here means the C port disagrees with the proven Python builder, not
 * with the oracle. Exit 0 on success, 1 on any mismatch.
 */
#include <stdio.h>
#include <string.h>

#include "adsp_init_image.h"

static int g_failures = 0;

static void check(int cond, const char *label, unsigned got, unsigned want)
{
    if (cond) {
        printf("PASS %s\n", label);
    } else {
        printf("FAIL %s: got 0x%08X want 0x%08X\n", label, got, want);
        g_failures++;
    }
}

int main(void)
{
    static uint32_t img[STUNRUN_ADSP_INIT_STATE_WORDS];
    static uint16_t dm[STUNRUN_ADSP_DM_SIZE];
    size_t n;
    size_t i;
    int all_zero = 1;

    /* Entry/origin contract shared by every fixture. */
    check(STUNRUN_ADSP_ORIGIN == 0x0000u, "origin-is-zero",
          STUNRUN_ADSP_ORIGIN, 0x0000u);
    check(STUNRUN_ADSP_ENTRY == 0x0004u, "reset-entry-is-0x0004",
          STUNRUN_ADSP_ENTRY, 0x0004u);

    /* nop fixture: 5 zero words, 20 bytes. */
    n = stunrun_adsp_emit(STUNRUN_ADSP_FIXTURE_NOP, img,
                          STUNRUN_ADSP_NOP_WORDS);
    check(n == 5u, "nop-length", (unsigned)n, 5u);
    for (i = 0; i < STUNRUN_ADSP_NOP_WORDS; i++)
        all_zero &= (img[i] == 0u);
    check(all_zero, "nop-all-zero", all_zero ? 1u : 0u, 1u);

    /* reset-loop: JUMP $0004 at the reset entry. */
    n = stunrun_adsp_emit(STUNRUN_ADSP_FIXTURE_RESET_LOOP, img,
                          STUNRUN_ADSP_RESET_LOOP_WORDS);
    check(n == 5u, "reset-loop-length", (unsigned)n, 5u);
    check(img[0x0004u] == 0x0018004Fu, "reset-loop-jump",
          img[0x0004u], 0x0018004Fu);

    /* init-prefix: observed reset calls plus RTS stubs. */
    n = stunrun_adsp_emit(STUNRUN_ADSP_FIXTURE_INIT_PREFIX, img,
                          STUNRUN_ADSP_INIT_PREFIX_WORDS);
    check(n == 0x835u, "init-prefix-length", (unsigned)n, 0x835u);
    check(img[0x0004u] == 0x001C780Fu, "init-prefix-call-0780",
          img[0x0004u], 0x001C780Fu);
    check(img[0x0005u] == 0x001C834Fu, "init-prefix-call-0834",
          img[0x0005u], 0x001C834Fu);
    check(img[0x0006u] == 0x0018006Fu, "init-prefix-loop",
          img[0x0006u], 0x0018006Fu);
    check(img[0x0780u] == 0x000A000Fu, "init-prefix-stub-0780",
          img[0x0780u], 0x000A000Fu);
    check(img[0x0834u] == 0x000A000Fu, "init-prefix-stub-0834",
          img[0x0834u], 0x000A000Fu);

    /* init-state: literal setup prefix spot checks. */
    n = stunrun_adsp_emit(STUNRUN_ADSP_FIXTURE_INIT_STATE, img,
                          STUNRUN_ADSP_INIT_STATE_WORDS);
    check(n == 0x847u, "init-state-length", (unsigned)n, 0x847u);
    check(img[0x0006u] == 0x00340008u, "init-state-setup-first",
          img[0x0006u], 0x00340008u);
    check(img[0x000Du] == 0x0038000Bu, "init-state-setup-nibble-last",
          img[0x000Du], 0x0038000Bu);
    check(img[0x000Fu] == 0x00380017u, "init-state-setup-ninth",
          img[0x000Fu], 0x00380017u);
    check(img[0x003Fu] == 0x0018043Fu, "init-state-mailbox-enter",
          img[0x003Fu], 0x0018043Fu);
    check(img[0x004Fu] == 0x001804D1u, "init-state-mailbox-tail",
          img[0x004Fu], 0x001804D1u);
    check(img[0x0050u] == 0x0018050Fu, "init-state-mailbox-loop",
          img[0x0050u], 0x0018050Fu);
    check(img[0x0780u] == 0x00380014u, "init-state-routine-0780-first",
          img[0x0780u], 0x00380014u);
    check(img[0x07A1u] == 0x000A000Fu, "init-state-routine-0780-rts",
          img[0x07A1u], 0x000A000Fu);
    check(img[0x0834u] == 0x00340014u, "init-state-routine-0834-first",
          img[0x0834u], 0x00340014u);
    check(img[0x0846u] == 0x000A000Fu, "init-state-routine-0834-rts",
          img[0x0846u], 0x000A000Fu);

    /* DM landmarks from analysis/ram-map.md. */
    memset(dm, 0, sizeof(dm));
    stunrun_adsp_apply_dm_landmarks(dm);
    check(dm[0x0955u] == 0x1242u, "dm-0955", dm[0x0955u], 0x1242u);
    check(dm[0x0956u] == 0x124Eu, "dm-0956", dm[0x0956u], 0x124Eu);
    check(dm[0x0959u] == 0x7FFFu, "dm-0959", dm[0x0959u], 0x7FFFu);
    check(dm[0x095Au] == 0xFFFFu, "dm-095a", dm[0x095Au], 0xFFFFu);

    /* Unknown fixture is a clean no-op, never a partial write. */
    check(stunrun_adsp_fixture_words((stunrun_adsp_fixture_t)99) == 0u,
          "unknown-fixture-words", 1u, 1u);

    if (g_failures == 0)
        printf("adsp init-slice C port: all checks passed\n");
    return g_failures == 0 ? 0 : 1;
}
