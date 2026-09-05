# Milestones

Milestones are defined by observable, testable behavior rather than code volume.

- [ ] **M0 — Reproducible baseline**: select one ROM set and MAME version; record hashes/commands; produce deterministic boot/title trace.
- [ ] **M1 — Machine map**: document processors, ROM/RAM/device regions, interrupts, and major communication paths to useful confidence.
- [ ] **M2 — Reproduction toolchain**: produce the smallest replacement/rebuilt image accepted by the original/emulated environment.
- [ ] **M3 — Reset and initialization**: reproduction reaches the expected early initialization path and stable loop/checkpoint.
- [ ] **M4 — Native shell**: Linux target has deterministic timing, input, rendering/logging scaffolding, and checkpoint support without speculative game logic.
- [ ] **M5 — First visible output**: reproduction matches the earliest useful visible frame/state from the original.
- [ ] **M6 — Title screen**: reproduction and native targets reach a title-screen-equivalent state; verifier has a canonical comparison.
- [ ] **M7 — Attract mode transition**: timing/state transition into attract behavior matches the original for the tested path.
- [ ] **M8 — Start input**: deterministic coin/start input produces the same high-level transition.
- [ ] **M9 — First gameplay frame**: player/gameplay state is initialized equivalently.
- [ ] **M10 — Player control**: a small scripted steering/input sequence produces matching state transitions.
- [ ] **M11 — Core movement**: player motion and relevant fixed-point/math behavior match over a bounded deterministic run.
- [ ] **M12 — First interaction**: collision/obstacle/enemy interaction is reproduced and differentially verified.

## Milestone completion rule

A milestone is complete only when Agent 3 can repeatably demonstrate the required behavior against the original reference. Visual resemblance alone is not sufficient when machine/game state can reasonably be compared.

## After M12

Do not pre-plan the entire game. Derive the next milestones from observed architecture and the highest-value unresolved subsystem. Likely areas include geometry submission, track/world progression, opponents/obstacles, scoring, sound, level transitions, and game-over flow.