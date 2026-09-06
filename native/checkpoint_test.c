/* ROM-free self-check for the checkpoint emitter contract.
 *
 * Populates the canonical M1 fixture values (transcribed from
 * reference/checkpoints/m1-machine-map/state.json — semantic equivalence
 * to that file is verified independently in tests/test_checkpoint_
 * golden.py) and checks the emitter contract: no truncation, exact-fit
 * sizing, determinism, and required keys. Prints the emitted document to
 * stdout for the golden gate. Exit 0 on success.
 */
#include <stdio.h>
#include <string.h>

#include "checkpoint.h"

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

static const uint32_t kFirstWords[STUNRUN_CHECKPOINT_FIRST_WORDS] = {
    655391u, 655391u, 655391u, 655391u,
    1865743u, 1868623u, 3407880u, 3407881u,
    3407882u, 3407883u, 3670024u, 3670025u,
    3670026u, 3670027u, 3407893u, 3670039u
};

static void fill_m1_fixture(stunrun_checkpoint_t *cp)
{
    unsigned i;
    memset(cp, 0, sizeof(*cp));
    strcpy(cp->system, "stunrun");
    strcpy(cp->description, "S.T.U.N. Runner (rev 6)");
    strcpy(cp->mame, "0.289 (mame0289-dirty)");
    cp->frame = 600;
    cp->time_seconds = 9.96648;
    strcpy(cp->maincpu_tag, ":mainpcb:maincpu");
    cp->maincpu_pc = 195726u;
    cp->maincpu_sr = 8196u;
    cp->maincpu_sp = 4294966838u;
    strcpy(cp->gsp_tag, ":mainpcb:gsp");
    cp->gsp_pc = 4294186624u;
    cp->gsp_st = 2097168u;
    strcpy(cp->adsp_tag, ":mainpcb:adsp");
    cp->adsp_pc = 20u;
    cp->adsp_astat = 0u;
    strcpy(cp->soundcpu_tag, ":mainpcb:jsa:cpu");
    cp->soundcpu_pc = 16724u;
    cp->soundcpu_p = 112u;
    strcpy(cp->region_space, "program");
    strcpy(cp->region_range, "0x0000-0x1fff");
    cp->region_word_width = 32;
    cp->region_nonzero_words = 2718u;
    cp->region_sum32 = 1636702025u;
    for (i = 0; i < STUNRUN_CHECKPOINT_FIRST_WORDS; i++)
        cp->region_first_words[i] = kFirstWords[i];
    cp->adsp_program_loaded = 1;
}

int main(void)
{
    static stunrun_checkpoint_t cp;
    static char doc[4096];
    static char doc2[4096];
    static char tiny[64];
    size_t len, len2, exact;
    fill_m1_fixture(&cp);

    /* Contract: NULL/empty inputs never produce a document. */
    check(stunrun_checkpoint_emit(NULL, doc, sizeof(doc)) == 0u);
    check(stunrun_checkpoint_emit(&cp, NULL, sizeof(doc)) == 0u);
    check(stunrun_checkpoint_emit(&cp, doc, 0u) == 0u);

    /* Contract: too-small buffers fail instead of truncating. */
    check(stunrun_checkpoint_emit(&cp, tiny, sizeof(tiny)) == 0u);

    /* Contract: exact-fit buffer succeeds. */
    len = stunrun_checkpoint_emit(&cp, doc, sizeof(doc));
    check(len > 0u && len < sizeof(doc));
    exact = len + 1u;
    {
        static char fit[2048];
        if (exact <= sizeof(fit))
            check(stunrun_checkpoint_emit(&cp, fit, exact) == len);
        else
            check(0);
    }

    /* Contract: deterministic across calls. */
    len2 = stunrun_checkpoint_emit(&cp, doc2, sizeof(doc2));
    check(len2 == len);
    check(memcmp(doc, doc2, len) == 0);

    /* Contract: required keys present in the fixed order. */
    check(strstr(doc, "\"schema\": \"stunrun-checkpoint/v1\"") != NULL);
    check(strstr(doc, "\"selectors\": {\"adsp_program_loaded\": true}") != NULL);
    check(strstr(doc, "\"nonzero_words\": 2718") != NULL);
    check(strstr(doc, "\"frame\": 600") != NULL);

    /* Emit the document for the golden gate. */
    if (g_failures == 0) {
        printf("--- document begins ---\n");
        fputs(doc, stdout);
        printf("--- document ends ---\n");
        printf("checkpoint emitter C port: all checks passed\n");
    }
    return g_failures == 0 ? 0 : 1;
}
