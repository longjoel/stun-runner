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
- `fifo_block.h` / `fifo_block.c` — length/terminator framing and literal
  transfer shape from the ADSP serial buffer (`0x810000`) to the GSP FIFO
  (`0xC0000C`). Payload semantics remain unresolved. Sources:
  `adsp-buffer-window.metadata.json` and `adsp-interface-map.metadata.json`.
- `trajectory_state.h` / `trajectory_state.c` — literal initialization,
  signed step, one-bit ROM-sourced delta filter, and clamp mechanism for the
  trajectory coordinate at `0xFFDCC6`. The physical axis and effect/object
  identity remain unknown; this is not an ammo or weapon-inventory model.
- `road_fifo.h` / `road_fifo.c` — complete and bounded partial forms of the
  observed base-buffer to GSP FIFO lane: every other source word is emitted.
  The range form models bursts split across adjacent frames without assigning
  payload semantics or frame scheduling.

The native target compiles these same sources (`native/CMakeLists.txt`:
`score-slice-c`, `nvram-scores-slice-c`, `fifo-block-slice-c`). Public
ROM-free checks: `tests/test_maincpu_c_score.py`,
`tests/test_maincpu_c_nvram.py`, `tests/test_maincpu_c_fifo_block.py`, and
the compiled self-checks `score_test.c` / `nvram_scores_test.c`.
