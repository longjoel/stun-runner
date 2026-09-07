/* Course-table march and twin-buffer fan-out, in C.
 *
 * Agent 2 (Implementer) encoding of Agent 1's frozen upload contract.
 * Literal mechanism only — table semantics, field meanings, and the
 * (course, segment) -> table-base map remain unresolved.
 *
 * Provenance (all OBSERVED-IN-TRACE unless noted):
 * - March readers 0x29760/0x2976E each read the same 384 words at +2
 *   step (768 reads per pass, 384 distinct addresses) over frames
 *   1286-1288: reference/experiments/stunrun/geometry-residue-capture.metadata.json
 *   (followup_session.rom_read_trace).
 * - Header 0x20074 = 0x0002 with 0x20076 = 0x22DA, read a handful of
 *   times each by PCs 0x29760-0x29782/0x298C0-0x298C4. The 0x0002 is an
 *   INFERENCE (two twin copies observed downstream), not a decoded
 *   count semantic; 0x22DA is UNKNOWN.
 * - March bases per course epoch (same PCs, same 384-word +2 shape):
 *   0x44630 (course 0), 0x45230 (course 5), 0x45530 (course 10),
 *   0x44930 (courses 11 and 12); all congruent 0x230 (mod 0x300),
 *   i.e. mutually 0x300-spaced slots: QUESTIONS.md IRQ-0005
 *   updates 8-9. No single course-proportional stride exists;
 *   courses share tables from the pool.
 * - Dual-copy fan-out at +0x300 stride with disjoint writer-PC groups
 *   per copy (base group never touches the twin copy and vice versa):
 *   same metadata (followup_session.findings). Destination base
 *   addresses are NOT frozen here; the caller supplies them.
 * - Saved-state producer fixture:
 *   reference/experiments/stunrun/m5-save-state-road-buffer-fixture.metadata.json
 *   captures both 768-byte destinations at 0xFF9584/0xFF9884 from a recorded
 *   race checkpoint; repeated forks are byte-identical and the copies match.
 *
 * Deliberately NOT claimed: what 0x22DA means; which (course, segment)
 * selects which table base (caller input); per-copy writer scheduling.
 *
 * Validated end to end (local oracle data, not embedded): the 384-word
 * table reconstructed from the traced 0x44630 march, run through
 * stunrun_geom_upload, matches work RAM verbatim, 768/768 bytes at both
 * 0xFF9584 and 0xFF9884 in the course-0 drive frame-1800 snapshot. The
 * same comparison at frames 1380/1500 scores 682/768 (upload still in
 * flight), and mid-flight frame-1320 snapshots match nowhere — the
 * model reproduces the settled upload, not partial states.
 *
 * No ROM contents are embedded or required.
 */
#ifndef STUNRUN_GEOM_UPLOAD_H
#define STUNRUN_GEOM_UPLOAD_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* March shape: 384 consecutive words stepped by 2 address units. */
#define STUNRUN_GEOM_MARCH_WORDS 384u
#define STUNRUN_GEOM_MARCH_STEP 2u

/* Header addresses and the observed count value. */
#define STUNRUN_GEOM_COUNT_ADDR 0x20074u
#define STUNRUN_GEOM_AUX_ADDR 0x20076u
#define STUNRUN_GEOM_COPY_COUNT 2u

/* March-reader PCs (both passes read the same table). */
#define STUNRUN_GEOM_MARCH_PC_A 0x29760u
#define STUNRUN_GEOM_MARCH_PC_B 0x2976Eu

/* Observed march-table bases (course, base); shared across courses. */
#define STUNRUN_GEOM_TABLE_C0 0x44630u
#define STUNRUN_GEOM_TABLE_C5 0x45230u
#define STUNRUN_GEOM_TABLE_C10 0x45530u
#define STUNRUN_GEOM_TABLE_C11_C12 0x44930u

/* Twin-copy stride in bytes between the two destination buffers. */
#define STUNRUN_GEOM_TWIN_STRIDE 0x300u

/* Observed destination bases for the course-0 early-segment upload
 * (single observation, same run family as the march trace):
 * reference/experiments/stunrun/geometry-residue-capture.metadata.json
 * (followup_session.upload_validation). The observed residue twin
 * ranges (0xFF9609/0xFF97A0 families and their +0x300 twins) fall
 * inside these extents. Whether every segment upload targets the same
 * bases is UNKNOWN. */
#define STUNRUN_GEOM_DEST_BASE 0xFF9584u
#define STUNRUN_GEOM_DEST_TWIN 0xFF9884u

/* One march pass: copy STUNRUN_GEOM_MARCH_WORDS words from table to
 * image in ascending address order. NULL pointers are safe no-ops. */
void stunrun_geom_march(const uint16_t *table, uint16_t *image);

/* Fan-out: copy the marched image to both destination buffers.
 * NULL pointers are safe no-ops. */
void stunrun_geom_fanout(const uint16_t *image, uint16_t *dst_a,
                         uint16_t *dst_b);

/* Full upload gated on the header count: exactly STUNRUN_GEOM_COPY_COUNT
 * performs both passes; any other count performs none (count semantics
 * beyond the observed 2 are UNKNOWN). Returns passes performed.
 * NULL pointers are safe no-ops returning 0. */
unsigned stunrun_geom_upload(unsigned count, const uint16_t *table,
                             uint16_t *dst_a, uint16_t *dst_b);

#ifdef __cplusplus
}
#endif

#endif /* STUNRUN_GEOM_UPLOAD_H */
