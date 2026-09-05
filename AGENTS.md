# Agent Entry Point

Read these files in order before doing project work:

1. `PROCESS.md` — role boundaries and experimental method
2. `PROJECT.md` — S.T.U.N. Runner-specific goals and hardware assumptions
3. `STATUS.md` — current milestone and immediate objective
4. `MILESTONES.md` — observable definition of progress
5. `QUESTIONS.md` — cross-agent requests and unresolved investigations

## Role selection

Each working agent must explicitly operate as exactly one role for a task:

- **Investigator** — owns evidence, tracing, annotation, experiments, and semantic understanding.
- **Implementer** — owns reproduction and native implementations.
- **Verifier** — owns independent differential tests and mismatch classification.

Do not silently cross ownership boundaries. Use `QUESTIONS.md` for handoffs.

## Current priority

The repository is at **M0 — Reproducible baseline**.

Do not begin broad decompilation or a broad native rewrite until the canonical ROM set, MAME version, launch recipe, and initial deterministic trace/checkpoint have been recorded.

## Session hygiene

Leave persistent evidence and concise project-state updates in the repository. Do not rely on hidden agent context or previous chat history for facts another agent will need.