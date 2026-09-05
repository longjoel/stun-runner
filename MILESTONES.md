# Milestones

Milestones are defined by observable, testable behavior rather than code volume.

- [ ] **M0 — Reproducible machine, harness, evidence, and toolchain baseline**: complete Step 0 driver mining against the pinned MAME revision; freeze one ROM set and MAME build; commit the MAME-derived hash manifest; validate local ROMs; reconcile runtime device tags with the mined machine map; generate stable static listings for every active programmable processor; establish the browser-automation-style MAME harness; run a deterministic bounded boot/title experiment; produce a canonical checkpoint and failure artifact bundle; prove bounded debugger tracing and trace-to-LCOV for at least one processor; and validate minimal code-emission paths for every active programmable CPU/DSP requiring replacement code. M0 is the preflight gate for substantial reverse-engineering work.
- [ ] **M1 — Machine map**: refine the Step 0 map with runtime evidence: processors, ROM/RAM/device regions, interrupts, communication paths, and initial inter-processor contracts to useful confidence using repeatable harness experiments.
- [ ] **M2 — Reproduction image/toolchain integration**: use the already-proven emission paths to produce the smallest replacement/rebuilt image accepted by the original/emulated environment. M2 proves image layout, linking/placement, and runtime integration; it does not defer basic assembler/encoder feasibility from M0.
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
Step 0 MAME driver mining
→ ROM validation
→ pinned MAME launch
→ runtime machine inventory
→ reconcile inventory with mined machine map
→ static listings + hardware landmarks
→ deterministic harness experiment
→ checkpoint/failure artifacts
→ bounded trace
→ original-code coverage report
→ minimal replacement-code emission proof for active targets
```

If any step still depends on an agent manually rediscovering source locations, debugger commands, device tags, paths, or undocumented setup, M0 is not complete.

Optional processors from a hardware family are not M0 toolchain requirements unless the target-specific pinned MAME configuration/runtime inventory demonstrates they are active.

## Milestone completion rule

A milestone is complete only when the Verifier can repeatably demonstrate the required behavior against the original reference. Visual resemblance alone is not sufficient when machine/game state can reasonably be compared.

## After M12

Do not pre-plan the entire game. Derive subsequent milestones from observed architecture and the highest-value unresolved subsystem. Likely areas include geometry submission, track/world progression, opponents/obstacles, scoring, sound, level transitions, and game-over flow.
