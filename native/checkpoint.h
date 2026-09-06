/* stunrun-checkpoint/v1 C representation and canonical emitter.
 *
 * Agent 2 (Implementer), M4 checkpoint support. The schema itself is
 * Verifier-owned (analysis/checkpoint-schema.md); this file only renders
 * it in C so both reconstruction targets can emit checkpoints the
 * Verifier can compare. Field set mirrors the canonical M1 fixture
 * reference/checkpoints/m1-machine-map/state.json exactly; no semantic
 * gameplay field is added (deliberately omitted per the schema doc,
 * IRQ-0003 still open).
 *
 * The emitter never truncates: a NULL or too-small buffer yields 0.
 * Output is deterministic for a given struct (fixed field order, fixed
 * float precision). Semantic equivalence of the golden output to the
 * oracle fixture is checked independently in tests/test_checkpoint_
 * golden.py with a real JSON parser — the C side pins the contract
 * (sizes, determinism, required keys), not the oracle values.
 *
 * No ROM contents are embedded or required.
 */
#ifndef STUNRUN_CHECKPOINT_H
#define STUNRUN_CHECKPOINT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STUNRUN_CHECKPOINT_SCHEMA "stunrun-checkpoint/v1"
#define STUNRUN_CHECKPOINT_FIRST_WORDS 16u

typedef struct stunrun_checkpoint {
    char system[32];
    char description[64];
    char mame[64];
    unsigned frame;
    double time_seconds;
    char maincpu_tag[32];
    uint32_t maincpu_pc;
    uint32_t maincpu_sr;
    uint32_t maincpu_sp;
    char gsp_tag[32];
    uint32_t gsp_pc;
    uint32_t gsp_st;
    char adsp_tag[32];
    uint32_t adsp_pc;
    uint32_t adsp_astat;
    char soundcpu_tag[32];
    uint32_t soundcpu_pc;
    uint32_t soundcpu_p;
    char region_space[16];
    char region_range[24];
    unsigned region_word_width;
    unsigned region_nonzero_words;
    uint32_t region_sum32;
    uint32_t region_first_words[STUNRUN_CHECKPOINT_FIRST_WORDS];
    int adsp_program_loaded;
} stunrun_checkpoint_t;

/* Render the checkpoint as canonical JSON into out (NUL-terminated).
 * Returns bytes written excluding the NUL, or 0 when cp/out is NULL or
 * the buffer is too small (never a truncated document). */
size_t stunrun_checkpoint_emit(const stunrun_checkpoint_t *cp,
                               char *out, size_t capacity);

#ifdef __cplusplus
}
#endif

#endif /* STUNRUN_CHECKPOINT_H */
