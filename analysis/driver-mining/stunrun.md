# Worked Step 0 Example — S.T.U.N. Runner

This document is the worked example for `DRIVER_MINING.md`.

It demonstrates how to mine MAME driver source for useful hardware puzzle pieces before broad disassembly or tracing.

## Provenance

Target:

```text
MAME shortname: stunrun
Driver family: src/mame/atari/harddriv.cpp
Related files: src/mame/atari/harddriv.h, harddriv_m.cpp, harddriv_v.cpp
Observed MAME source revision during this mining pass: a263f49884249c87b18d8b61f007dc24004e450b
```

This is a **source-mining snapshot**, not yet the project's canonical pinned MAME revision. Re-run this pass after the project pins MAME for M0.

## 1. Immediate machine-model correction

### Prior project assumption

The provisional machine model included a second TMS34010-class MSP.

### MAME finding

S.T.U.N. Runner's target-specific board configuration calls:

```cpp
multisync_nomsp(config);
```

Evidence: `MAME-CONFIRMED`.

### Interpretation

The current MAME configuration does **not** instantiate the optional MSP for S.T.U.N. Runner.

This is exactly the kind of family-level assumption Step 0 is intended to catch.

### Project action

Treat the active programmable-processor inventory as:

- Motorola 68010 main CPU;
- TMS34010 GSP;
- ADSP-2100;
- 6502-class JSA II sound CPU.

Do not include a second TMS34010 MSP unless later runtime inventory or pinned-MAME configuration contradicts this finding.

The TMS34012/PSP remains a graphics device that must be inventoried, but should not automatically be treated as an independently programmed software target.

---

## 2. Target initialization path

MAME's S.T.U.N. Runner initializer performs:

```cpp
init_multisync(0);
init_adsp();
```

Evidence: `MAME-CONFIRMED`.

This establishes two high-value subsystem boundaries immediately:

```text
main board / Multisync graphics
ADSP math/geometry board
```

The target machine configuration also instantiates Atari JSA II sound and wires its main interrupt callback into the Hard Drivin' family main-CPU sound interrupt path.

Evidence: `MAME-CONFIRMED`.

---

## 3. ROM-region puzzle pieces

`ROM_START(stunrun)` defines a 1 MiB region tagged:

```text
mainpcb:maincpu
```

for the main 68010 program image.

The first pair is loaded byte-interleaved at even/odd addresses beginning at `0x000000` / `0x000001`.

Example first pair from the mined MAME revision:

```text
136070-2102.200r
  offset: 0x000000
  size:   0x010000
  CRC32:  e0ed54d8
  SHA1:   15850568d8308b6499cbe55b5d8308041d906a29

136070-2101.210r
  offset: 0x000001
  size:   0x010000
  CRC32:  3008bcf8
  SHA1:   9d3a20b639969bab68441f76467ed60e395c10e3
```

Evidence: `MAME-CONFIRMED`.

### Why this matters

The listing/reconstruction tooling must honor the interleaving rather than treating each physical ROM file as an independent linear 68010 image.

The final authoritative ROM manifest must come from the pinned MAME build/listxml, not from this worked example.

---

## 4. Main CPU → ADSP windows

`init_adsp()` dynamically installs 68010-visible handlers.

### ADSP program window

```text
68010 address: 0x800000–0x807fff
access: read/write
MAME handlers:
  hd68k_adsp_program_r
  hd68k_adsp_program_w
```

Evidence: `MAME-CONFIRMED`.

Candidate selector:

```text
adsp.program
```

Candidate symbol family:

```text
ADSP_PROGRAM_WINDOW
```

### ADSP data window

```text
68010 address: 0x808000–0x80bfff
access: read/write
MAME handlers:
  hd68k_adsp_data_r
  hd68k_adsp_data_w
```

Evidence: `MAME-CONFIRMED`.

Candidate selector:

```text
adsp.data
```

Candidate symbol family:

```text
ADSP_DATA_WINDOW
```

### Important semantic caveat

The handler names are `MAME-NAMED-HYPOTHESIS` for game-level meaning. The addresses and handler installation are `MAME-CONFIRMED`; the purpose of individual locations inside those windows must be learned from runtime behavior.

---

## 5. High-value first ADSP experiment

### Question

Does the 68010 dynamically populate ADSP program memory during boot, and if so, which 68010 routines perform the upload?

### Experiment

```text
start from power-on
watch all writes to 0x800000–0x807fff
for each write capture:
  frame/cycle identity
  68010 PC
  target address
  written value
  nearby register state
stop after initialization/title checkpoint
```

Then:

1. dump the resulting ADSP program memory;
2. disassemble the runtime image;
3. correlate contiguous upload loops with main-CPU XREFs;
4. compare the runtime image to any ROM/serial source regions present in MAME.

This experiment should become a reusable harness spec, not a one-off debugger session.

---

## 6. Input landmarks

MAME's S.T.U.N. Runner input definition explicitly documents:

```text
mainpcb:IN0
68010-visible address comment: 0x60c000
```

Bit `0x01` is the diagnostic jumper in the mined definition.

Evidence: `MAME-CONFIRMED` for MAME's modeled input mapping.

Candidate selector:

```text
input.in0
```

### Static-analysis use

The Investigator should search the 68010 listing for references to `0x60c000` before broad function naming.

Those XREFs are candidate input-polling or input-state routines.

### Differential experiment

Capture two short traces differing only in one control state and compare the execution/dataflow around XREFs to `0x60c000`.

---

## 7. ADC/input landmarks

The driver input definitions contain ADC address comments. One documented example is:

```text
0xb80000 — 12-bit ADC channel 3
```

Evidence: `MAME-CONFIRMED` as a modeled input landmark.

The same input block contains a comment that S.T.U.N. Runner has its own coin handling rather than simply using the ordinary JSA II coin definition.

This is valuable because it tells the Investigator not to assume the generic JSA coin path is the entire coin/start story for this title.

### Candidate experiments

- enumerate all S.T.U.N. Runner ADC fields through the Lua inventory harness;
- correlate analog input changes with reads of the documented ADC ranges;
- search static 68010 XREFs to each ADC base address;
- compare coin insertion against both main-board input and JSA traffic.

---

## 8. Sound boundary

The S.T.U.N. Runner machine config instantiates:

```text
ATARI_JSA_II
```

and connects its main interrupt callback to the Hard Drivin' family sound interrupt handler.

Evidence: `MAME-CONFIRMED`.

### Early reconstruction implication

For early gameplay work, treat JSA II as a subsystem boundary.

The first semantic objective is not to reverse engineer YM2151 synthesis. It is to understand:

```text
68010 command submission
JSA acknowledgement/interrupt behavior
6502 command consumption
```

Then the reproduction/native targets can model or bridge that boundary incrementally.

---

## 9. Candidate static landmarks

The first static listing pass should prioritize XREFs to:

```text
0x60c000              main input landmark
0x800000–0x807fff     ADSP program interface
0x808000–0x80bfff     ADSP data interface
0xb80000 family       ADC/input landmarks
```

These XREFs should be annotated with provenance-aware labels immediately.

Example:

```asm
move.w  ADSP_PROGRAM_WINDOW+0x12,d0
```

is preferable to leaving the raw absolute address unexplained, but the label must remain a hardware-interface label, not an inferred gameplay meaning.

---

## 10. Candidate harness selectors

Initial low-level selectors that can be defined before game semantics are understood:

```text
machine.maincpu
machine.gsp
machine.adsp
machine.soundcpu

input.in0
adsp.program
adsp.data
```

Selectors for player position, game state, enemies, etc. must wait for runtime evidence.

---

## 11. Assumption conflicts discovered by Step 0

### Conflict DM-0001 — optional MSP

```text
Prior assumption:
  S.T.U.N. Runner actively uses a second TMS34010 MSP.

MAME evidence:
  target configuration calls multisync_nomsp(config).

Current resolution:
  remove MSP from active processor inventory for this target unless pinned-MAME runtime inventory contradicts the current source revision.
```

This should be reflected in `PROJECT.md`, `REQUIREMENTS.md`, M0 toolchain acceptance, and coverage expectations.

---

## 12. Unknowns intentionally left open

Driver mining does **not** establish:

- exact game-level meaning of locations inside ADSP data RAM;
- which 68010 routines correspond to player/game state;
- whether every documented ADC channel is actively used by S.T.U.N. Runner;
- exact GSP command/data protocol used by the game;
- exact JSA command semantics;
- semantic ownership of particular interrupts;
- runtime timing requirements for DSP/GSP synchronization.

These remain runtime investigation targets.

---

## 13. First experiment queue derived from the driver

### DM-EXP-001 — ADSP upload

Watch `0x800000–0x807fff` from power-on through title.

Goal: identify upload code and obtain runtime ADSP program image.

### DM-EXP-002 — ADSP data traffic

Change-only telemetry for `0x808000–0x80bfff` around initialization and first gameplay.

Goal: locate command/status/shared-data hot spots.

### DM-EXP-003 — main input XREF

Find static references to `0x60c000`, then compare traces with one digital input toggled.

Goal: identify input polling/normalization path.

### DM-EXP-004 — ADC differential

Vary one analog input at a time and correlate reads from documented ADC addresses.

Goal: map physical controls to program-visible values and reader PCs.

### DM-EXP-005 — sound command boundary

Trigger a deterministic sound event and capture 68010/JSA communication plus interrupt behavior.

Goal: identify sound command submission and acknowledgement protocol.

---

## 14. What future agents should copy from this example

Do not copy S.T.U.N. Runner addresses to another game.

Copy the method:

```text
find target config
→ identify actual instantiated devices
→ find init-time dynamic handlers
→ extract maps/inputs/ROM layout
→ seed provenance-aware constants/selectors
→ compare against assumptions
→ generate bounded runtime experiments
```

The point of Step 0 is to begin reverse engineering with a box of labeled puzzle pieces instead of dumping the entire puzzle onto the table face-down.
