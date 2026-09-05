# MAME Instrumentation and Telemetry

This project should prefer stock MAME plus Lua/debugger scripting before considering a custom MAME build.

The goal is to make the emulator act as a reusable laboratory harness that can enumerate the machine, drive deterministic experiments, capture state, and emit evidence in stable formats.

## Operating principle

Do not patch MAME merely to obtain data that the stock Lua/debugger APIs can already expose.

Prefer this order:

1. stock MAME command-line options;
2. Lua `-autoboot_script` instrumentation;
3. Lua plugins where persistent reusable tooling is useful;
4. debugger commands invoked directly or through Lua;
5. only then consider a custom MAME build.

Pin the exact MAME version/commit because the Lua API is not guaranteed stable across releases.

## What stock MAME Lua can provide

Without recompiling MAME, Lua can be used for:

- MAME/application version and current-machine metadata;
- device-tree enumeration;
- debugger-visible CPU/device discovery;
- register/state enumeration and reads/writes;
- memory-space enumeration;
- memory reads and writes;
- memory region/share/bank enumeration;
- screen enumeration, geometry, and frame counts;
- per-frame and periodic callbacks;
- input/I/O port discovery and manipulation;
- session pause/reset/stop control;
- graphical telemetry overlays;
- debugger command execution when `-debug` is enabled;
- debugger breakpoints/watchpoints and controlled stepping;
- save-state/snapshot/dump/disassembly commands through the debugger.

This is enough to build most of the project's first-generation oracle and telemetry harness around an unmodified MAME executable.

## Recommended reusable script layout

```text
mame/
  lua/
    lib/
      json.lua
      filesystem.lua
      machine.lua
      devices.lua
      memory.lua
      inputs.lua
      telemetry.lua
      experiments.lua
      debugger.lua
    inventory.lua
    capture.lua
    replay.lua
    watch.lua
    coverage.lua
  scripts/
    debugger/
      listings.cmd
      maps.cmd
  experiments/
    boot_to_title.yaml
    coin_start.yaml
```

The `lib/` layer should be game-agnostic. S.T.U.N. Runner-specific addresses, tags, checkpoints, and inputs belong in configuration/experiment files rather than generic Lua helpers.

## Script 1: machine inventory

`inventory.lua` should be runnable against any MAME target and emit a machine-readable inventory.

Capture at least:

- MAME version/API version;
- machine shortname;
- complete device tree;
- device tag, short name, type/name where exposed;
- devices exposing debugger interfaces;
- register/state symbols for each processor;
- address spaces for each memory-capable device;
- memory regions, shares, and banks;
- screens and frame counters;
- input ports and fields.

Output conceptually:

```json
{
  "schema": 1,
  "mame": "0.xxx",
  "system": "stunrun",
  "devices": {
    ":mainpcb:maincpu": {
      "spaces": ["program"],
      "states": ["PC", "D0", "D1"]
    }
  }
}
```

No per-game assumptions should be necessary for inventory generation.

## Script 2: telemetry sampler

`telemetry.lua` should provide generic sampling of configured registers and memory locations at a selectable cadence.

Support at least:

- every frame;
- every N frames;
- periodic emulated-time interval where appropriate;
- one-shot checkpoint capture;
- change-only logging.

Configuration concept:

```yaml
sample:
  every_frames: 1
signals:
  - name: main_pc
    device: ":mainpcb:maincpu"
    state: PC
  - name: game_state
    device: ":mainpcb:maincpu"
    space: program
    address: 0x1234
    width: 16
```

Output should be JSON Lines or another streamable structured format rather than free-form console text.

## Script 3: deterministic replay

`replay.lua` should translate the project's canonical replay schema into MAME input actions.

It should:

- begin from a declared start condition;
- apply digital and analog controls at deterministic frame boundaries;
- emit a log of applied events;
- capture declared checkpoints;
- terminate automatically at success/failure/timeout.

The logical replay schema must remain target-independent so the same experiment can later drive reproduction and native builds.

## Script 4: checkpoint capture

`capture.lua` should create a checkpoint bundle from stock MAME.

Where supported through Lua directly or by invoking debugger commands, capture:

- frame number and emulated time;
- selected CPU registers/state;
- configured memory regions/ranges;
- screen snapshot;
- machine/device inventory identity;
- ROM-manifest identity supplied by the harness;
- MAME version/API version;
- experiment/replay identifier;
- hashes of emitted binary dumps.

When `-debug` is active, Lua may call debugger commands to perform operations such as disassembly, memory-map dumps, binary saves, screen snapshots, and save states.

## Script 5: execution coverage collector

Initial execution coverage should remain debugger/trace based unless Lua/debugger hooks provide a reliable lower-overhead mechanism for a given processor.

The generic coverage pipeline should accept either:

- MAME debugger trace output; or
- address-hit events collected through breakpoints/watchpoints/other debugger facilities for bounded experiments.

Normalize both into:

```text
processor tag
address
hit count
experiment id
```

Then feed the existing MAME-to-LCOV conversion described in `LCOV.md`.

Do not make a generic Lua callback per instruction unless measured overhead is acceptable; stock debugger tracing is the baseline for complete instruction streams.

## Script 6: watch/experiment helper

`watch.lua` should make focused experiments cheap.

Desired generic operations:

- watch a memory range for writes;
- break/log when a value changes;
- log selected registers when a breakpoint/watchpoint fires;
- count breakpoint hits;
- patch/freeze a configured memory value for an experiment;
- stop automatically after a trigger or timeout;
- emit an experiment result file with method, observation, and raw event data.

This becomes the Investigator's reusable laboratory kit.

## Debugger integration from Lua

When MAME is started with `-debug`, Lua can access the debugger manager and CPU debugger interfaces.

This allows generic scripts to:

- execute debugger console commands;
- set/remove breakpoints;
- set/remove watchpoints where exposed;
- step CPUs;
- resume execution;
- inspect debugger console/error logs.

This means a Lua harness can orchestrate existing debugger functionality instead of duplicating it.

For example, the harness can generate static listings using debugger `dasm`, memory maps with `memdump`, binary ranges with `save`, screenshots with `snap`, and save states with `statesave`.

## Stock MAME limitations

Do not assume Lua exposes every internal event with zero overhead.

Potential reasons to consider deeper instrumentation later include:

- instruction-by-instruction callbacks with lower overhead than debugger tracing;
- internal renderer/polygon-command details not exposed through public memory/device state;
- bus transactions not represented as readable shared state;
- device-specific internal queues or signals not exposed to Lua;
- extremely high-volume telemetry where debugger/Lua overhead perturbs timing or produces impractical data volume.

These are later optimization problems. First prove what can be learned through stable observable interfaces.

## Generic tooling acceptance criteria

Before broad reverse engineering, the generic MAME harness should demonstrate:

- [ ] inventory generation works on S.T.U.N. Runner without hard-coded device tags;
- [ ] inventory identifies every programmable processor used by the game;
- [ ] a Lua script can read selected registers and memory every frame;
- [ ] deterministic input replay can drive a small experiment;
- [ ] a checkpoint bundle can be generated automatically;
- [ ] Lua can invoke the debugger to emit at least one listing and memory-map dump;
- [ ] a bounded trace can be initiated/stopped reproducibly;
- [ ] output metadata contains MAME/API version and experiment identity;
- [ ] generated artifacts conform to versioned schemas;
- [ ] the same generic harness can be pointed at another MAME game with only configuration changes.

## Credit-control rule

Agents should build reusable instrumentation before repeatedly performing manual debugger work.

If an agent discovers a useful observation procedure that will likely be repeated, the preferred output is not just the resulting trace. It is:

```text
reusable script/config
+ reproducible command
+ resulting evidence
```

The emulator laboratory should become progressively more automated as the investigation proceeds.
