# Step 0 — MAME Driver Mining

Before disassembling a target, tracing broad execution, or asking an agent to infer architecture from raw machine code, mine the pinned MAME driver and related device implementation for explicit machine facts and useful hypotheses.

This is **Step 0** for every future arcade reverse-engineering target.

The purpose is not to treat MAME source as ground truth about game semantics. The purpose is to extract the emulator author's already-known hardware facts so agents do not waste time rediscovering addresses, device wiring, ROM layouts, or board boundaries from scratch.

## Core rule

MAME driver source is authoritative for **what the pinned MAME build models**. It is not automatically authoritative for **what the original game program means**.

Every extracted fact must retain provenance and a confidence/evidence label.

Use:

- `MAME-CONFIRMED` — explicit in the pinned MAME configuration, map, ROM definition, or handler wiring;
- `MAME-NAMED-HYPOTHESIS` — MAME handler/member naming suggests semantic meaning, but game behavior has not independently verified it;
- `BOARD-DOCUMENTED` — supported by schematics/manuals/board documentation;
- `OBSERVED-IN-TRACE` — directly confirmed from runtime evidence;
- `ASSUMED` — project hypothesis not yet verified.

Do not silently promote `MAME-NAMED-HYPOTHESIS` into game semantics.

---

## Required Step 0 inputs

Pin or record:

- MAME release/version;
- MAME git commit when available;
- target shortname/set;
- driver source file(s);
- target-specific state/device class;
- related machine/device implementation files;
- ROM definitions;
- address maps;
- input-port definitions;
- initialization functions;
- machine configuration functions.

If the MAME revision changes later, regenerate or version-scope the mined output.

---

## Mining pass

### 1. Locate the target registration and ROM definition

Find:

- `ROM_START(target)`;
- parent/clone relationship;
- ROM regions and device tags;
- load offsets, widths, interleaving, CRC32, SHA-1;
- auxiliary PROM/EEPROM/PLD regions where relevant.

Outputs:

- ROM manifest inputs;
- program/data region ownership;
- byte-lane/interleave rules;
- candidate executable images.

### 2. Locate target-specific machine configuration

Find the target's `machine_config` or board-device configuration and record:

- CPUs/DSPs actually instantiated;
- devices explicitly omitted;
- device tags;
- clock values;
- sound/video subsystems;
- callback/interrupt wiring;
- watchdog/timer/device connections.

This step overrides assumptions derived from generic board-family comments.

A family may support optional processors that a specific title does not instantiate.

### 3. Locate initialization path

Find target-specific init functions and common initializers they invoke.

Record:

- dynamic memory handlers installed at runtime;
- ROM/RAM banking setup;
- protection/slapstic configuration;
- DSP program/data windows;
- device-specific initialization;
- patches/hacks/workarounds present in MAME.

Runtime-installed handlers are especially important because they may not appear in the static address-map function.

### 4. Mine CPU/DSP address maps

For every programmable processor, extract:

- ROM ranges;
- RAM ranges;
- shared RAM;
- device registers;
- command/status mailboxes;
- palette/framebuffer/video RAM;
- input ranges;
- sound interfaces;
- interrupt acknowledge/control ranges;
- protection/watchdog/NVRAM;
- unknown/nop ranges worth observing.

Represent the result as machine-readable regions plus human commentary.

### 5. Mine input definitions

Extract:

- port tags;
- documented CPU-visible addresses from comments/maps;
- digital bits;
- analog channels and widths;
- coin/start/service/test inputs;
- diagnostic jumpers and DIP switches;
- active-high/active-low polarity.

These become immediate harness inputs and static-disassembly landmarks.

### 6. Mine interrupts and inter-device wiring

Record explicit callback connections such as:

```text
sound board interrupt -> main CPU interrupt handler
GSP interrupt -> main CPU line
ADSP interrupt/flag -> main CPU status/IRQ path
```

Treat handler names as hypotheses until runtime confirms behavior.

### 7. Mine named handlers and constants

Collect meaningful MAME names such as:

```text
adsp_program
adsp_data
sound_data
irq_ack
palette
shared_ram
```

Use them to seed **candidate symbols and harness selector names**.

Do not annotate original game routines with those semantics solely because MAME uses the name.

### 8. Record contradictions against project assumptions

Driver mining must explicitly compare findings against existing project documentation.

Example:

```text
ASSUMED: board has optional MSP processor
MAME-CONFIRMED: target calls multisync_nomsp()
ACTION: remove MSP from active target inventory unless runtime evidence contradicts MAME
```

Contradictions are high-value outputs, not inconveniences.

### 9. Turn constants into first experiments

For each high-value range, propose a concrete experiment.

Examples:

```text
ADSP program window -> watch all main-CPU writes during boot
ADSP data window    -> change-only telemetry during first gameplay frame
input register      -> static XREF search + input-differential trace
sound command       -> watch writes around a known sound event
shared RAM          -> record writer PC and reader processor
```

Driver mining is complete only when useful facts have been converted into candidate selectors, symbols, or experiments.

---

## Required outputs

Create:

```text
analysis/driver-mining/<target>.md
analysis/driver-mining/<target>.machine-map.yaml
```

The Markdown document should contain:

- source provenance;
- active machine inventory;
- ROM-region summary;
- address landmarks;
- interrupt/device relationships;
- input landmarks;
- candidate symbols/selectors;
- contradictions/unknowns;
- proposed first experiments.

The machine-readable file should contain stable facts that tooling can consume.

Suggested schema shape:

```yaml
schema: 1
target: game
mame:
  commit: ...
processors:
  - id: maincpu
    type: m68010
    evidence: MAME-CONFIRMED
regions:
  - name: dsp_program_window
    processor: maincpu
    start: 0x800000
    end: 0x807fff
    access: rw
    evidence: MAME-CONFIRMED
    candidate_selector: adsp.program
```

Addresses must be numeric values, not prose strings, in the actual machine-readable artifact.

---

## Agent behavior after Step 0

### Investigator

Before broad annotation:

1. consume the mined machine map;
2. verify active device tags against running MAME;
3. search static listings for XREFs to mined address landmarks;
4. execute the proposed bounded experiments;
5. promote facts to `OBSERVED-IN-TRACE` only when runtime evidence supports them.

### Implementer

Use mined hardware boundaries to establish interfaces and toolchain targets, but do not implement game semantics from MAME handler names alone.

### Verifier

Turn mined constants into stable low-level harness selectors and use runtime inventory to reject stale driver-mining data when the pinned MAME build does not match.

---

## Automation goal

This process should eventually be partially automated.

A future `tools/mame-driver-mine` helper may extract candidate facts from MAME source/listxml and generate a draft machine map for agent review.

Automation must not erase provenance or uncertainty. The useful product is:

```text
source fact -> normalized puzzle piece -> runtime experiment -> verified project knowledge
```

not a generated document pretending source-code names are semantic truth.

---

## Definition of done

Step 0 is complete for a target when:

- [ ] target ROM/config/init locations are identified;
- [ ] active programmable devices are listed;
- [ ] optional family devices not used by the target are explicitly excluded;
- [ ] known address ranges and input landmarks are recorded;
- [ ] inter-device/interrupt wiring is summarized;
- [ ] candidate symbols/selectors are seeded with provenance;
- [ ] contradictions against prior assumptions are recorded;
- [ ] at least one bounded runtime experiment is proposed for every major processor boundary;
- [ ] machine-readable output exists;
- [ ] the Investigator can start static XREF and dynamic trace work from these puzzle pieces rather than from an empty slate.
