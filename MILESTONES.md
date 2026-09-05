# Milestones

Milestones are defined by observable, testable behavior rather than code volume.

- [ ] **M0 — Reproducible machine, harness, evidence, and toolchain baseline**: freeze one ROM set and pinned MAME build; commit the MAME-derived hash manifest; validate the local ROMs; enumerate debugger-visible processor/device tags; generate stable static listings for every programmable processor; establish the browser-automation-style MAME harness; run a deterministic bounded boot/title experiment; produce a canonical checkpoint and failure artifact bundle; prove bounded debugger tracing and trace-to-LCOV for at least one processor; and validate a minimal code-emission path for 68010, 6502, TMS34010, and ADSP-2100. M0 is the preflight gate for substantial reverse-engineering work.
- [ ] **M1 — Machine map**: document processors, ROM/RAM/device regions, interrupts, major communication paths, and initial inter-processor contracts to useful confidence using repeatable harness experiments.
- [ ] **M2 — Reproduction image/toolchain integration**: use the already-proven per-processor emission paths to produce the smallest replacement/rebuilt image accepted by the original/emulated environment. M2 proves image layout, linking/placement, and runtime integration; it does not defer basic assembler/encoder feasibility from M0.
- [ ] **M3 — Reset and initialization**: reproduction reaches the expected early initialization path and stable loop/checkpoint.
- [ ] **M4 — Native shell**: Linux target has deterministic timing, input, rendering/logging scaffolding, harness-compatible replay/checkpoint support, and no speculative game logic.
- [ ] **M5 — First visible output**: reproduction matches the earliest useful visible frame/state from the original.
- [ ] **M6 — Title screen**: reproduction and native targets reach a title-screen-equivalent state; Verifier has a canonical comparison.
- [ ] **M7 — Attract mode transition**: timing/state transition into attract behavior matches the original for the tested path.
- [ ] **M8 — Start input**: deterministic coin/start input through the common replay contract produces the same high-level transition.
- [ ] **M9 — First gameplay frame**: player/gameplay state is initialized equivalently.
- [ ] **M10 — Player control**: a small scripted steering/input sequence produces matching state transitions.
- [ ] **M11 — Core movement**: player motion and relevant fixed-point/math behavior match over a bounded deterministic run.
- [ ] **M12 — First interaction**: collision/obstacle/enemy interaction is reproduced and differentially verified.

## M0 completion rule

M0 is complete only when the repository can repeat the preflight sequence without ad hoc operator knowledge:

```text
ROM validation
→ pinned MAME launch
→ machine inventory
→ deterministic harness experiment
→ checkpoint/failure artifacts
→ bounded trace
→ original-code coverage report
→ minimal replacement-code emission proof
```

If any of those steps still depends on an agent manually rediscovering debugger commands, device tags, paths, or undocumented setup, M0 is not complete.

## Milestone completion rule

A milestone is complete only when Agent 3 can repeatably demonstrate the required behavior against the original reference. Visual resemblance alone is not sufficient when machine/game state can reasonably be compared.

## After M12

Do not pre-plan the entire game. Derive the next milestones from observed architecture and the highest-value unresolved subsystem. Likely areas include geometry submission, track/world progression, opponents/obstacles, scoring, sound, level transitions, and game-over flow.