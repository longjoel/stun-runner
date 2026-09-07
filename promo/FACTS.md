# S.T.U.N. RUNNER — REVERSE-ENGINEERING PROJECT

**One line:** We are resurrecting Atari's 1989 tunnel-racer S.T.U.N. Runner
as verified reconstruction + native Linux port, with MAME as the
behavioral oracle and evidence required for every claim.

## Why it's cool

- **Four brains, one game.** A Motorola 68010, a TMS34010 graphics
  processor, an ADSP-2100 geometry DSP, and a 6502 sound CPU all
  cooperate every frame. We are interrogating each one.
- **No guessing allowed.** Every semantic claim needs a trace, a
  checkpoint, or a replay to back it. `UNKNOWN` is a valid result.
- **Tiny verified slices beat big rewrites.** Milestones M0–M4 are done;
  M5 ("first visible output") is the current quest.

## Scoreboard (per `STATUS.md`)

- M0 — reproducible machine, harness, evidence baseline: **complete**
- M1 — machine/interconnect contract + input-dependent render boundary: **complete**
- M2 — replacement ADSP image executes in MAME: **complete**
- M3 — source-defined ADSP init slice, repeatable loop: **complete**
- M4 — deterministic native timing/replay/checkpoint shell: **complete**
- Canonical set: S.T.U.N. Runner (rev 6), 25 ROM files, MAME-verified
- Title checkpoint: frame 600 / 9.97 emulated seconds
- Test suite: 12 CTest targets + 55 ROM-free repo tests passing

## The method (steal this)

MAME driver mining → hardware puzzle pieces → reproducible oracle
experiment → evidence-backed fixture → semantic understanding →
reproduction → native port → independent verification.

## Links to drop in

- `PROJECT.md` — goals and architecture
- `STATUS.md` — current milestone and objective
- `MILESTONES.md` — what "done" looks like
- `analysis/driver-mining/stunrun.md` — Step 0 worked example
