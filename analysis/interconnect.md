# Interconnect Map — M1 Working Record

This file is the Investigator-owned M1 record. It intentionally begins with
unknowns; MAME handler names and address ranges identify hardware interfaces,
not game-level semantics.

## MAME-confirmed starting points

| Interface | Evidence | Current claim |
|---|---|---|
| 68010 program space | `analysis/driver-mining/stunrun.machine-map.yaml` | Active main CPU at `:mainpcb:maincpu`. |
| ADSP program window | same machine map; `hd68k_adsp_program_r/w` | 68010-visible range `0x800000–0x807fff`. |
| ADSP data window | same machine map; `hd68k_adsp_data_r/w` | 68010-visible range `0x808000–0x80bfff`. |
| JSA interrupt callback | Step 0 driver-mining record | Sound board is wired into the main-board interrupt path; runtime behavior remains to be measured. |
| MSP | runtime reconciliation | No active MSP device is present for this target. |

## Observed-in-trace

No M1 interconnect observation has been promoted yet. The six-frame M0 trace
metadata explicitly records empty GSP/ADSP traces as a bounded observation,
not proof of inactivity.

## Open contracts

### 68010 ↔ ADSP

- UNKNOWN: whether and when the 68010 uploads program words during boot.
- UNKNOWN: which data-window offsets are commands, status, or shared data.
- UNKNOWN: interrupt/flag direction and acknowledgement sequence.

### 68010 ↔ GSP/PSP

- UNKNOWN: first runtime GSP execution boundary for the title/start path.
- UNKNOWN: command submission region and synchronization mechanism.

### 68010 ↔ JSA sound board

- UNKNOWN: command register/queue offsets and acknowledgement behavior.
- UNKNOWN: which deterministic input/event is the smallest useful sound trigger.

## Evidence queue

The bounded experiment definitions and results belong under `reference/`; the
interpretation of those results belongs here. Start with the ADSP upload slice
specified in `analysis/m1-plan.md`, then update this file with evidence links,
confidence labels, and narrowed unknowns.
