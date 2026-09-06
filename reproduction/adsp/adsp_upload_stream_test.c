/* ROM-free self-check for the 68010 upload-stream C contract.
 *
 * Every expectation mirrors a checked-in value from
 * reference/experiments/stunrun/adsp-program-upload.metadata.json. The
 * table transcription is verified computationally (sums, contiguity, gap,
 * address rule), not by re-reading the header. Exit 0 on success.
 */
#include <stdio.h>

#include "adsp_upload_stream.h"

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
    const stunrun_adsp_upload_record_t *recs = stunrun_adsp_upload_records();
    unsigned total;
    unsigned gaps = 0;
    size_t i;

    check(stunrun_adsp_upload_record_count() ==
          STUNRUN_ADSP_UPLOAD_RECORD_COUNT);
    check(STUNRUN_ADSP_UPLOAD_RECORD_COUNT == 17u);

    /* Transcribed table sums to the metadata's 2728-word total, which is
     * exactly half the 5456 observed halfword taps (one MOVE.L each). */
    total = stunrun_adsp_upload_total_words();
    check(total == STUNRUN_ADSP_UPLOAD_PROGRAM_WORDS);
    check(total == 2728u);
    check(2u * total == STUNRUN_ADSP_UPLOAD_HALFWORD_TAPS);

    /* Records are contiguous except for exactly one gap, and the gap is
     * the metadata's explicit 2203-4136 (1934 words). */
    for (i = 1; i < stunrun_adsp_upload_record_count(); i++) {
        unsigned expect =
            recs[i - 1].destination_word_index +
            recs[i - 1].program_word_count;
        if (recs[i].destination_word_index != expect) {
            gaps++;
            check(recs[i - 1].destination_word_index == 2100u);
            check(expect == STUNRUN_ADSP_UPLOAD_GAP_FIRST);
            check(recs[i].destination_word_index ==
                  STUNRUN_ADSP_UPLOAD_GAP_LAST + 1u);
        }
    }
    check(gaps == 1u);
    check(STUNRUN_ADSP_UPLOAD_GAP_LAST - STUNRUN_ADSP_UPLOAD_GAP_FIRST +
          1u == STUNRUN_ADSP_UPLOAD_GAP_WORDS);
    check(STUNRUN_ADSP_UPLOAD_GAP_WORDS == 1934u);

    /* Gap membership and record ownership. */
    check(!stunrun_adsp_upload_in_gap(2202u));
    check(stunrun_adsp_upload_in_gap(2203u));
    check(stunrun_adsp_upload_in_gap(3000u));
    check(stunrun_adsp_upload_in_gap(4136u));
    check(!stunrun_adsp_upload_in_gap(4137u));
    check(stunrun_adsp_upload_record_for_word(0u) == 0);
    check(stunrun_adsp_upload_record_for_word(2202u) == 14);
    check(stunrun_adsp_upload_record_for_word(2203u) == -1);
    check(stunrun_adsp_upload_record_for_word(3000u) == -1);
    check(stunrun_adsp_upload_record_for_word(4136u) == -1);
    check(stunrun_adsp_upload_record_for_word(4137u) == 15);
    check(stunrun_adsp_upload_record_for_word(4661u) == 16);
    check(stunrun_adsp_upload_record_for_word(4662u) == -1);

    /* Address rule reproduces the observed coverage: first transfer at
     * 0x800000, final MOVE.L at 0x8048D4 with its last halfword tap at
     * the observed 0x8048D6. */
    check(stunrun_adsp_upload_word_address(0u) ==
          STUNRUN_ADSP_UPLOAD_OBSERVED_FIRST);
    check(stunrun_adsp_upload_word_address(4661u) + 2u ==
          STUNRUN_ADSP_UPLOAD_OBSERVED_LAST);
    check(stunrun_adsp_upload_word_address(4661u) == 0x8048D4u);
    check(stunrun_adsp_upload_word_address(0x2000u) == 0u);

    /* Source-pointer contract: bus halves match the observed 0x0001 /
     * 0x702E pair and the pointer sits inside 68010 ROM. */
    check(stunrun_adsp_upload_source_ptr_hi() == 0x0001u);
    check(stunrun_adsp_upload_source_ptr_lo() == 0x702Eu);
    check(STUNRUN_ADSP_UPLOAD_SOURCE_PTR >= STUNRUN_ADSP_UPLOAD_ROM_FIRST);
    check(STUNRUN_ADSP_UPLOAD_SOURCE_PTR <= STUNRUN_ADSP_UPLOAD_ROM_LAST);

    /* Writer PC lies inside the transfer loop range. */
    check(STUNRUN_ADSP_UPLOAD_WRITER_PC >= STUNRUN_ADSP_UPLOAD_LOOP_FIRST);
    check(STUNRUN_ADSP_UPLOAD_WRITER_PC <= STUNRUN_ADSP_UPLOAD_LOOP_LAST);

    /* Terminator and control constants. */
    check(STUNRUN_ADSP_UPLOAD_CONTROL_TERMINATOR == 0xFFu);
    check(STUNRUN_ADSP_UPLOAD_CONTROL_DATA == 0x00u);

    if (g_failures == 0)
        printf("adsp upload-stream C port: all checks passed\n");
    return g_failures == 0 ? 0 : 1;
}
