# MAME Harness — Playwright for Arcade Machines

This project treats MAME as an executable runtime that should be automated the way web developers automate browsers with Playwright or Selenium.

The harness is not merely a collection of Lua snippets. It is the project's stable automation layer for driving the original game, collecting evidence, running deterministic experiments, and producing repeatable test artifacts.

## Core analogy

```text
Browser runtime       -> MAME machine
DOM / page state      -> machine memory, CPU state, devices, screen
Selector              -> semantic machine-state selector
Click / type          -> input injection
Navigation            -> boot / reset / load state
Screenshot            -> frame capture
Browser trace         -> CPU/DSP traces + telemetry + input timeline
Fixture               -> canonical save state / checkpoint
Test spec              -> deterministic experiment
Assertion              -> state / framebuffer / audio / timing comparison
Coverage               -> native LCOV + original execution LCOV
```

The harness should make common tests readable at the behavior level while preserving low-level access for investigation.

## Architecture

Keep three layers separate:

```text
Test specifications / experiments
          ↓
Stable arcade automation API
          ↓
MAME Lua + debugger adapter
          ↓
Stock MAME
```

Tests should not depend directly on unstable MAME Lua details unless they are explicitly low-level instrumentation tests.

If MAME changes its Lua API, the adapter changes; the project experiment/test API should remain stable.

## Proposed reusable objects

The game-agnostic harness should expose concepts similar to:

```text
ArcadeSession
Machine
Cpu
MemorySpace
Input
Screen
Trace
Checkpoint
Fixture
Selector
Expectation
Experiment
ArtifactBundle
```

The exact Lua surface may evolve, but these conceptual boundaries should remain stable.

## High-level API

Behavioral tests should prefer high-level operations:

```lua
local arcade = require("arcade")

local game = arcade.launch("stunrun")

game:boot()
game:wait_until("title", { timeout_frames = 1800 })
game:press("COIN1")
game:wait_frames(30)
game:press("START1")
game:expect_state("gameplay")
game:checkpoint("first-gameplay-frame")
```

High-level operations should include at least:

- `boot()` / reset;
- load/save fixture state;
- `wait_frames(n)`;
- bounded `wait_until(selector)`;
- press/release/hold digital controls;
- set analog controls;
- capture checkpoint;
- screenshot;
- start/stop bounded trace;
- assertions against semantic or low-level state;
- automatic diagnostic artifact capture on failure.

## Low-level escape hatch

Reverse engineering requires direct access below the behavioral API.

Low-level objects should expose operations conceptually like:

```lua
cpu:register("PC")
memory:read_u16(address)
memory:write_u16(address, value)
debugger:breakpoint(address)
debugger:watchpoint(address, length, "write")
debugger:trace_start(...)
```

These are valid for:

- investigation;
- fault injection;
- rare-state setup;
- hypothesis experiments;
- fixture construction;
- debugging failures.

Normal E2E tests should prefer observable inputs and outputs over directly forcing internal state.

## Semantic selectors

A major goal is to create selector semantics analogous to Playwright locators.

Early selectors may be concrete machine facts:

```yaml
selectors:
  title:
    all:
      - memory:
          device: ":mainpcb:maincpu"
          space: program
          address: 0x1234
          width: 16
          equals: 3

  gameplay:
    all:
      - memory:
          device: ":mainpcb:maincpu"
          address: 0x1234
          width: 16
          equals: 7
```

As knowledge improves, selectors may become semantic abstractions such as:

```text
state("title")
state("gameplay")
object("player")
object("enemy", 3)
mailbox("adsp")
region("game_state")
```

Selectors are part of the semantic model and must retain evidence/provenance. A selector must not silently change merely to make a reconstructed implementation pass.

## Wait semantics

All asynchronous waits must be bounded.

Never wait indefinitely for a condition.

Example:

```lua
game:wait_until("title", {
  timeout_frames = 1800
})
```

A failed wait should automatically capture enough diagnostics to explain what happened.

At minimum:

- current frame and emulated time;
- relevant selector values;
- screenshot;
- selected register/memory snapshot;
- input timeline;
- last bounded trace window when enabled.

## Assertions

Assertions should exist at multiple levels.

Behavioral:

```text
expect_state("gameplay")
expect_screen("title")
expect_object("player").x == ...
```

Machine-level:

```text
expect_memory(...)
expect_register(...)
expect_region_hash(...)
expect_mailbox(...)
```

Differential:

```text
expect_checkpoint_matches(original, reproduction)
expect_checkpoint_matches(original, native, normalization="gameplay")
```

The test definition must state whether equality is exact or normalized.

## Fixtures

Steal Playwright's fixture concept directly.

A fixture establishes a known starting state that many tests can reuse.

Conceptually:

```lua
fixture("title_screen", function(game)
  game:boot()
  game:wait_until("title")
  game:save_state("title_screen")
end)
```

Then:

```lua
test.use("title_screen")

test("start enters gameplay", function(game)
  game:press("START1")
  game:expect_state("gameplay")
end)
```

Fixtures should reduce repeated boot/setup work and make tests fast, but fixture provenance must include the pinned ROM and MAME build.

Never let save-state fixtures become untracked magic binaries. Their generation recipe and identity belong in metadata.

## Trace-on-failure

Like Playwright traces, failed tests should produce a compact artifact bundle automatically.

Example:

```text
artifacts/
  coin-start-failure/
    metadata.json
    input.jsonl
    screenshot.png
    checkpoint-before.json
    checkpoint-after.json
    telemetry.jsonl
    maincpu-last-5000.trace
    coverage.info
```

Do not capture maximum-volume telemetry for every successful run when it is unnecessary.

The harness should support policies such as:

```text
trace: retain-on-failure
screenshots: only-on-failure
telemetry: rolling-buffer
coverage: always
```

A rolling trace/telemetry buffer is desirable so a failure can retain the preceding N frames/instructions without writing enormous logs throughout the test.

## Deterministic experiment format

The same logical experiment must be usable across:

- original MAME oracle;
- reproduction build under MAME;
- native implementation.

Example:

```yaml
schema: arcade-experiment/v1
id: coin_start
start:
  fixture: title_screen

events:
  - frame: 0
    control: COIN1
    action: press
  - frame: 1
    control: COIN1
    action: release
  - frame: 30
    control: START1
    action: press
  - frame: 31
    control: START1
    action: release

expect:
  selector: gameplay
  timeout_frames: 600
```

The harness adapter translates this into MAME input actions. The native runner later consumes the same experiment definition.

## Test runner

The eventual command-line experience should be intentionally boring and familiar.

Conceptual commands:

```text
arcade-test test
arcade-test test specs/stunrun/boot.lua
arcade-test test --experiment coin_start
arcade-test inventory stunrun
arcade-test record coin_start
arcade-test trace boot_to_title
arcade-test coverage
arcade-test report
```

For this repository these may initially be shell/Python/Lua scripts or `just`/CMake targets. The API boundary matters more than the first CLI implementation.

## Proposed directory layout

```text
mame/
  lua/
    arcade/
      session.lua
      machine.lua
      cpu.lua
      memory.lua
      input.lua
      screen.lua
      trace.lua
      checkpoint.lua
      selectors.lua
      expect.lua
      fixtures.lua
      artifacts.lua
      runner.lua
    adapter/
      mame.lua

  experiments/
    boot_to_title.yaml
    coin_start.yaml
    first_gameplay.yaml

  selectors/
    stunrun.yaml

  fixtures/
    metadata/

tests/
  oracle/
  integration/
  fixtures/
```

The `arcade/` layer should be reusable on other MAME targets. S.T.U.N. Runner-specific selectors and experiments must remain outside the generic library.

## Black-box default

Behavioral verification should use player-visible inputs whenever practical.

Prefer:

```text
press START
wait for gameplay
assert state
```

over:

```text
write game_state = gameplay
```

Direct state mutation remains a legitimate experimental tool, but it must be explicit and should not masquerade as an ordinary end-to-end test.

## Harness ownership

The Verifier owns the stability and semantics of the high-level test API, fixtures, assertions, failure artifacts, and replay execution.

The Investigator contributes selectors, telemetry definitions, and evidence-backed interpretations needed by tests.

The Implementer consumes the same experiment/checkpoint contracts for reproduction and native targets and must not redefine them privately.

## M0 acceptance criteria

Before broad reverse engineering begins, the harness should demonstrate all of the following against the original S.T.U.N. Runner machine:

- [ ] launch/pin/validate the target machine;
- [ ] enumerate devices automatically;
- [ ] run a test through a stable harness entry point;
- [ ] inject at least one deterministic control sequence;
- [ ] implement at least one bounded semantic or machine-state wait;
- [ ] implement at least one assertion;
- [ ] create/reuse one fixture or canonical start state;
- [ ] capture a checkpoint bundle;
- [ ] automatically retain diagnostics for a deliberately failing test;
- [ ] start/stop one bounded MAME debugger trace;
- [ ] feed one trace into original-code LCOV reporting;
- [ ] allow the same logical experiment schema to be consumed independently of MAME Lua internals.

## Strategic goal

The long-term output is not only a S.T.U.N. Runner decompilation workflow.

It is a reusable browser-automation-style test driver for emulated machines:

> Playwright for arcade software.

If a reverse-engineering procedure is useful repeatedly, it should become a harness capability rather than remain an agent ritual.
