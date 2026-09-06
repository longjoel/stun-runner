# 68010 main-CPU gameplay-state coverage (Agent 2 reproduction slices)

Literal, evidence-backed C models of frozen 68010-side findings. Every
module cites its source metadata and confidence; nothing here claims
semantics Agent 1 left open.

- `score.h` / `score.c` — live-score accumulator (`0xFF9532` longword),
  award selection (`0x03A2E2`: 500 when the course value is nonzero,
  else 50), the add (`0x03A2EC`), clear, and the object-hit counter
  (`0xFFDD02` via `0x03A298`). Golden vectors: the weapon_probe run
  (ten +50 awards, cumulative 50..500, high word zero) and the center
  run (selection branches, 34 counter increments to `0x22`). Sources:
  `main-ram-score-candidate.metadata.json` (DYNAMIC-SCORE-CONFIRMED)
  and `main-ram-center-object-score.metadata.json`
  (OBJECT_HIT_AND_SCORE_CONFIRMED). Other static addition sites,
  counter width/wrap, and score-to-table promotion are not modeled.
- `nvram_scores.h` / `nvram_scores.c` — persistent high-score table
  decoder for the ZRAM view (`0xFF4410`, ten 24-byte records, BE u16
  score at +0, name bytes at +2). The 10-entry golden test asserts
  exact score bytes and the recorded name prefix only: name
  termination/padding is unestablished and the padding filler is
  arbitrary. Source: `main-nvram-high-score-table.metadata.json`.

The native target compiles these same sources (`native/CMakeLists.txt`:
`score-slice-c`, `nvram-scores-slice-c`). Public ROM-free checks:
`tests/test_maincpu_c_score.py`, `tests/test_maincpu_c_nvram.py`, and
the compiled self-checks `score_test.c` / `nvram_scores_test.c`.
