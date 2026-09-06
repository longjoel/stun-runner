# Masked-snapshot geometry probe (runbook)

For the ROM-holder / Verifier. Goal: find where track geometry comes
from by snapshotting 68010 work RAM at title and under sustained drive,
diffing, and masking every range frozen evidence already explains. The
residue is the geometry-candidate set. Serves IRQ-0004 (cursor hunt) and
IRQ-0005 (stride differential): residue ranges are what those questions
trace next.

Status: recipe and tooling are committed and ROM-free tested. The capture
recipe has now been executed against the local validated ROM set; the
provenance record is
`reference/experiments/stunrun/geometry-residue-capture.metadata.json`.

## Captures (run each twice; residue counts only on identical repeats)

Title baseline, 600 frames, no input:

```sh
tools/mame-memory-snapshot /path/to/private/stunrun-roms /tmp/geom-title \
  --device :mainpcb:maincpu --space program \
  --base 0xFF8000 --count 0x80000 --width 8 \
  --frames 600 --input none --nothrottle
```

Drive state, 1800 frames, sustained steer (coin 650/680, start 750/780,
stick 220 plus Button 1 held from 900 — the `late_drive` schedule baked
into `mame/lua/memory-snapshot.lua`):

```sh
tools/mame-memory-snapshot /path/to/private/stunrun-roms /tmp/geom-drive \
  --device :mainpcb:maincpu --space program \
  --base 0xFF8000 --count 0x80000 --width 8 \
  --frames 1800 --input late_drive --nothrottle
```

Each capture writes `<output>/snapshot-<frame>.json` plus `result.json`
with per-snapshot SHA-256. The work-RAM window is the pinned
`0xFF8000–0xFFFFFF` region from
`analysis/driver-mining/stunrun.machine-map.yaml`.

## Mask

```sh
tools/mask-memory-snapshot --mask analysis/known-regions.json \
  --snapshot /tmp/geom-drive/snapshot-1800.json \
  --baseline /tmp/geom-title/snapshot-600.json \
  --output /tmp/geom-masked
```

This prints one summary line
(`interesting/explained/tracked/residue` counts) and writes
`residue.json` (`stunrun-masked-snapshot/v1`): coalesced residue ranges
with address, byte length, and sample values, plus `tracked` ranges
(known-consumer traffic with unresolved ownership: object records,
structure candidate) reported separately, never silently dropped. Empty
residue is a finding — record it, do not re-run with looser settings.

## Reading the residue

- A residue range that advances monotonically across several `--targets`
  frames is a cursor candidate for IRQ-0004: run
  `tools/mame-ram-write-trace` over it to find the mover PC, then check
  what ROM region that PC reads (IRQ-0005 stride test).
- A residue range holding small integers that step with the course word
  `0xFF9578` is a table-index candidate: diff drive captures across
  courses if a multi-course schedule exists.
- Residue inside `0xFF4000–0xFF4FFF` despite the ZRAM mask means the
  manifest is wrong: stop and report, do not extend the mask to cover
  it without new evidence.
- Do not add residue ranges to `analysis/known-regions.json` until a
  follow-up experiment promotes them; the mask records explanations,
  not observations.

## Classifying the residue (pipeline)

Six small tools annotate `residue.json` in place (same schema plus a
`tags` entry per range and a `classifiers` list, so stages chain in any
order). Every stage prioritizes follow-ups; none identifies a range —
promotion to an explanation still requires trace evidence:

```sh
# behavior over frames (needs --targets captures; monotonic-* feeds IRQ-0004)
tools/classify-temporal --residue r/residue.json \
  --snapshots title/snapshot-600.json drive/snapshot-1800.json \
  --output r/tagged-temporal
# writer attribution (needs a write-trace result over the ranges)
tools/classify-writers --residue r/tagged-temporal/residue.json \
  --trace traced/result.json --output r/tagged-writers
# mode discrimination across runs (drive-specific ranges first)
tools/classify-modes --residues r/drive.json r/weapon.json \
  --labels drive weapon --output r/tagged-modes
# struct-adjacency hints against the mask
tools/classify-adjacency --residue r/residue.json \
  --mask analysis/known-regions.json --output r/tagged-adjacency
# content-shape hints from snapshot bytes (tiny/sparse/dense/periodic/ascii)
tools/classify-shape --residue r/residue.json \
  --snapshot drive/snapshot-1800.json --output r/tagged-shape
# field-width inference from a diff pair (byte-exact, width-8 snapshots)
tools/classify-width --residue r/residue.json \
  --before title/snapshot-600.json --after drive/snapshot-1800.json \
  --output r/tagged-width
```

Triage order for the annotated ranges: monotonic behavior plus
single-writer plus drive-specific plus dense/periodic shape is the
strongest geometry-table signature the pipeline can express; anything
it flags still needs the writer-PC to ROM-read confirmation before it
enters the mask or the machine map. All six stages are covered by
`tests/test_classifiers.py` on synthetic fixtures, including an
end-to-end chaining test.

## Initial capture result

Two independent title captures produced the same frame-600 snapshot hash.
Two independent `late_drive` captures produced identical snapshot hashes at
relative frames 600, 1200, and 1800. Relative frame 600 is identical to the
title baseline because the drive input begins later. Masked baseline
comparisons found zero changed cells at frame 600, then 675 interesting cells
(662 residue, 9 tracked) at frame 1200 and 2,989 interesting cells (2,956
residue, 29 tracked) at frame 1800. The largest late residue ranges are in
`0xFF95xx–0xFF9Bxx` display/progression neighborhoods already seen in prior
RAM differentials. This capture does not identify a monotonic track cursor or
ROM record stride; those remain open for writer/read tracing.

The classifier pass over the same real capture series tagged 33 ranges as
monotonic-up and 9 as monotonic-down, but these are byte/range behaviors over
only three samples. Writer attribution resolves the most tempting periodic
fields as elapsed/animation state rather than a track cursor; see
`reference/experiments/stunrun/geometry-writer-attribution.metadata.json`.

## Recording

Write the run up as `reference/experiments/stunrun/` metadata
(`stunrun-experiment-metadata/v1`: runner commands, MAME revision,
repeatable-run hashes, residue summary with counts — never ROM
contents), keep the multi-megabyte snapshot JSONs local, and answer the
relevant IRQ-0004/0005/0006 items in `QUESTIONS.md`.
