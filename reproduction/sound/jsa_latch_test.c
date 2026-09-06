/* ROM-free self-check for the JSA latch-transport C model.
 *
 * Every expectation mirrors a checked-in value from
 * sound-boundary-tap.metadata.json or sound-handler-search.metadata.json.
 * Exit 0 on success.
 */
#include <stdio.h>

#include "jsa_latch.h"

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
    stunrun_jsa_latches_t latches;
    stunrun_jsa_title_counts_t counts;
    const stunrun_jsa_command_read_t *reads = stunrun_jsa_command_reads();

    /* Register classification from the MAME mirror mapping. */
    check(stunrun_jsa_classify_read(0x2802u) == STUNRUN_JSA_READ_COMMAND);
    check(stunrun_jsa_classify_read(0x280Au) == STUNRUN_JSA_READ_COMMAND);
    check(stunrun_jsa_classify_read(0x280Cu) == STUNRUN_JSA_READ_RDIO);
    check(stunrun_jsa_classify_read(0x280Eu) == STUNRUN_JSA_READ_IRQ_ACK);
    check(stunrun_jsa_classify_read(0x2810u) == STUNRUN_JSA_READ_OTHER);
    check(stunrun_jsa_is_response_reg(0x2A02u));
    check(!stunrun_jsa_is_response_reg(0x2A04u));

    /* Command-byte extraction from the frame-447 bus event only. */
    check(stunrun_jsa_command_byte(STUNRUN_JSA_FRAME447_BUS_DATA,
                                   STUNRUN_JSA_FRAME447_BUS_MASK) ==
          STUNRUN_JSA_FRAME447_BYTE);
    check(stunrun_jsa_command_byte(0x1E1Eu, 0x00FFu) == STUNRUN_JSA_NO_BYTE);
    check(stunrun_jsa_command_byte(0x1E1Eu, 0xFFFFu) == STUNRUN_JSA_NO_BYTE);

    /* Main-to-sound walkthrough with the frame-447 byte: write latches,
     * NMI asserts, the sound-side read returns 0x1E and clears NMI. */
    stunrun_jsa_latches_init(&latches);
    check(stunrun_jsa_sound_command_read(&latches) == STUNRUN_JSA_NO_BYTE);
    stunrun_jsa_main_command_write(&latches, STUNRUN_JSA_FRAME447_BYTE);
    check(latches.command_pending);
    check(latches.nmi_asserted);
    check(stunrun_jsa_sound_command_read(&latches) == 0x1Eu);
    check(!latches.command_pending);
    check(!latches.nmi_asserted);
    check(stunrun_jsa_sound_command_read(&latches) == STUNRUN_JSA_NO_BYTE);

    /* Sound-to-main walkthrough with the startup byte: write latches,
     * IRQ4 asserts, the handler-side read returns 0xFF and clears IRQ4. */
    stunrun_jsa_latches_init(&latches);
    check(stunrun_jsa_main_response_read(&latches) == STUNRUN_JSA_NO_BYTE);
    stunrun_jsa_sound_response_write(&latches, STUNRUN_JSA_STARTUP_BYTE);
    check(latches.response_pending);
    check(latches.irq4_asserted);
    check(stunrun_jsa_main_response_read(&latches) == 0xFFu);
    check(!latches.response_pending);
    check(!latches.irq4_asserted);

    /* A fresh command does not disturb a pending response and vice versa:
     * the two latches are independent (MAME models separate latches). */
    stunrun_jsa_latches_init(&latches);
    stunrun_jsa_main_command_write(&latches, 0x1Eu);
    stunrun_jsa_sound_response_write(&latches, 0xFFu);
    check(stunrun_jsa_sound_command_read(&latches) == 0x1Eu);
    check(latches.irq4_asserted);
    check(stunrun_jsa_main_response_read(&latches) == 0xFFu);
    check(!latches.nmi_asserted);

    /* NULL latches are safe no-ops. */
    stunrun_jsa_latches_init(NULL);
    stunrun_jsa_main_command_write(NULL, 0x1Eu);
    stunrun_jsa_sound_response_write(NULL, 0xFFu);
    check(stunrun_jsa_sound_command_read(NULL) == STUNRUN_JSA_NO_BYTE);
    check(stunrun_jsa_main_response_read(NULL) == STUNRUN_JSA_NO_BYTE);

    /* Title-path counts table: exact match, single-field perturbation,
     * and NULL rejection. */
    counts = stunrun_jsa_observed_title_counts();
    check(counts.main_command == 1u);
    check(counts.main_response == 4u);
    check(counts.sound_command == 4u);
    check(counts.sound_response == 3u);
    check(counts.sound_rdio == 283775u);
    check(counts.sound_irq_ack == 2487u);
    check(counts.sound_oki == 1244u);
    check(counts.sound_voice == 0u);
    check(counts.sound_wrio == 2504u);
    check(counts.sound_mix == 6u);
    check(counts.sound_other == 0u);
    check(stunrun_jsa_title_counts_match(&counts));
    counts.sound_mix = 7u;
    check(!stunrun_jsa_title_counts_match(&counts));
    check(!stunrun_jsa_title_counts_match(NULL));

    /* Observed $280A command-read sequence ending in the frame-447 byte. */
    check(reads[0].frame == 1u && reads[0].pc == 0x4139u &&
          reads[0].data == 0x00u);
    check(reads[1].frame == 369u && reads[1].data == 0x00u);
    check(reads[2].frame == 411u && reads[2].data == 0x00u);
    check(reads[3].frame == STUNRUN_JSA_FRAME447_FRAME &&
          reads[3].pc == STUNRUN_JSA_FRAME447_READ_PC &&
          reads[3].data == STUNRUN_JSA_FRAME447_BYTE);
    check(STUNRUN_JSA_COMMAND_READ_COUNT == 4u);

    if (g_failures == 0)
        printf("jsa latch-transport C port: all checks passed\n");
    return g_failures == 0 ? 0 : 1;
}
