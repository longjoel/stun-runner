# Cross-Agent Questions

Use this file for questions that cross agent ownership boundaries. Preserve resolved requests as project history.

## IRQ-0001

Status: OPEN
From: Project bootstrap
To: Investigator
Priority: HIGH

### Question

Which exact MAME S.T.U.N. Runner shortname/revision should be the canonical baseline for this repository?

### Required output

Record:

- shortname;
- displayed revision/title;
- ROM region/hash summary sufficient to identify the set without redistributing ROM data;
- MAME version/commit;
- exact launch command;
- any flags/scripts needed for deterministic tracing.

### Why it matters

All traces, snapshots, annotations, reproduction builds, and verifier comparisons need a single stable reference revision.

---

## IRQ-0002

Status: OPEN
From: Project bootstrap
To: Investigator
Priority: HIGH

### Question

What is the smallest useful processor/device map for S.T.U.N. Runner's boot and title path?

### Required output

Identify, to appropriate confidence:

- main 68010 role;
- ADSP role and communication path;
- graphics processor/device roles;
- sound command path;
- major shared-memory/mailbox regions;
- reset and interrupt flow relevant to the first milestone.

### Why it matters

Agent 2 needs to know which hardware behavior must be reproduced immediately and which can initially be treated as an external service.

---

## IRQ-0003

Status: OPEN
From: Project bootstrap
To: Verifier
Priority: MEDIUM

### Question

What is the minimum canonical checkpoint schema that can be captured repeatably from MAME and later emitted by both reconstruction targets?

### Desired properties

Prefer a small format that can grow over time. Candidate fields include frame/cycle identity, selected CPU registers, hashes of important RAM regions, named semantic values once known, and optional rendered-frame hashes.

### Why it matters

Verification should exist before substantial reconstruction so mismatches become evidence rather than subjective debugging sessions.