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

- Two independent 600-frame `adsp_program_upload` runs successfully registered
  a 68010 program-space write watchpoint over `0x800000–0x807fff`, but the
  installed headless MAME did not deliver watchpoint action callbacks. See
  `reference/experiments/stunrun/adsp-program-upload.metadata.json`.
- The watchpoint result is setup-only; it does **not** establish an access
  count, ADSP activity, or the program's load/source path.
- The six-frame M0 trace metadata still records empty GSP/ADSP instruction
  traces as a bounded observation, not proof of inactivity.
- OBSERVED-IN-TRACE: independent 60-frame traces for both GSP and ADSP are
  empty, while independent 600-frame traces are nonempty. The first observed
  GSP trace instruction is at `0xFFF59920`; the first observed non-NOP ADSP
  instruction is `0x0004: CALL $0780`. This bounds first activity to frames
  61–600 without identifying the exact frame. Independent repeats of both
  600-frame traces are byte-identical.
- See `reference/experiments/stunrun/processor-trace-600.metadata.json` for
  trace hashes and sizes.
- Two independent 600-frame coin/start runs registered read/write probes for
  the ADSP data window `0x808000–0x80bfff`, but headless MAME did not deliver
  their action callbacks; see `reference/experiments/stunrun/adsp-data-window.metadata.json`.

## Open contracts

### 68010 ↔ ADSP

- OBSERVED-IN-TRACE: no 68010 writes to the ADSP program window during two
  bounded 600-frame boot/title runs.
- UNKNOWN: whether the ADSP program is preloaded, loaded by another path, or
  populated outside this window.
- UNKNOWN: 68010 reads or writes to the ADSP data window during the bounded
  coin/start title-path runs, because the headless watchpoint callback path is
  unavailable.
- UNKNOWN: which data-window offsets are commands, status, or shared data, and
  whether access occurs only after a verified gameplay transition.

### 68010 input/status path

- OBSERVED-IN-TRACE: normal coin/start reaches the input checks at `0x02C1A0`
  and `0x02C1EE`.
- OBSERVED-IN-TRACE: an early Coin 2 variant enters a bounded trace dominated
  by `0x0013EC` → `0x0013FC`/`0x001404`, polling `0x60C001`.
- OBSERVED-IN-REPLAY: the normal and early Coin 1/start sequences both reach
  the same road/demo image as no input at frame 600, with `Credits: 0`.
- STATIC-CANDIDATE: routine `0x043590` is the only listing routine that samples
  coin bits 7/6 at `0x60C001`; it has six direct callers, but none executes in
  the tested boot/title/service/coin-start traces.
- UNKNOWN: whether the normal Coin 1 path leaves the displayed credit count at
  zero because the run remains in attract/demo, because a required transition
  is missing, or because the installed MAME input model differs from the ROM's
  expected environment. The current evidence does not justify treating the
  input checks as a credit-state contract.
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
