# Current Status

## Current milestone

**M0 — Establish a reproducible original-machine baseline**

## State

Original in MAME: **not yet baselined in this repository**

Reproduction target: **not started**

Native target: **not started**

Verification harness: **not started**

## Immediate objective

Choose one S.T.U.N. Runner MAME ROM set and one MAME version/commit, then produce a deterministic launch-and-trace recipe that reaches a stable title/attract state.

The first useful output is not decompiled C. It is a repeatable experiment.

## First handoff

### Agent 1 — Investigator

Establish:

- exact MAME shortname and ROM revision;
- exact MAME build/version;
- launch command;
- CPU/device inventory for that machine configuration;
- main CPU reset/vector entry path;
- a bounded boot trace;
- initial memory-map notes;
- title/attract checkpoint.

Record uncertainties explicitly.

### Agent 2 — Implementer

Until the baseline exists, avoid broad game implementation. Prepare only minimal scaffolding that does not encode unverified game semantics.

Once Agent 1 has identified the initial boot path, begin with the smallest reproduction artifact that proves the toolchain and image layout are correct.

### Agent 3 — Verifier

Define a compact canonical checkpoint format and determine which original-machine state can be captured repeatably from MAME at reset and at title/attract mode.

## Current blockers

- No canonical ROM set/revision selected yet.
- No canonical MAME version recorded yet.
- No deterministic trace recipe checked into the repository yet.

## Last verified checkpoint

None yet.