# Provisional game-loop model

This is the current evidence-backed model of the S.T.U.N. Runner runtime loop.
It is a mechanism model, not a claim that every RAM field has a final
gameplay name. The model should be updated as new writer/consumer traces close
the remaining boundaries.

## Summary

The strongest current interpretation is:

```text
hardware interrupt(s)
  -> increment global tick at 0xFF8014
  -> main 68010 state dispatch
       -> sample input and update input/state records
       -> update timers, speed, trajectory, and indexed object records
       -> build road/object/display commands
       -> drain buffered road data to the GSP FIFO
       -> issue sound commands when events occur

ADSP geometry worker --GINT -> main IRQ2 -> acknowledge
GSP command/FIFO worker -> VRAM primitives -> scanout
JSA 6502 worker --response IRQ4 -> main response handler
```

The exact top-level scheduler and the exact interrupt source for every tick
are not yet isolated. The loop below is therefore a useful implementation
boundary, not a decompiled function.

## Evidence-backed phases

### 1. Timebase and interrupt service

`0x0222DE` executes `addq.l #1,$8014.w` and returns through an `RTE`. A
runtime write trace at the bus-visible address `0xFF8014` captured 34,064
updates from frames 600–4799, with about four updates per video frame. This
makes `0xFF8014` a global interrupt tick, not a proven spatial track cursor.

The tick is copied or transformed into downstream timing inputs:

- `0xFFDCC0` — trajectory/update mirror;
- `0xFFDB36` — road/FIFO timing landmark;
- `0xFFDDEC`/`0xFFDDF0` — object-event timing inputs;
- shifted forms — placement/geometry helpers.

Evidence: `analysis/track-position-findings.md`,
`reference/experiments/stunrun/m5-global-tick-trace.metadata.json`.

### 2. Main state dispatch and input sampling

`0xFF9550` is a state/dispatch latch. The routine at `0x02452C` dispatches
through it, but the complete state table is not yet named.

The strongest input-path candidate is:

```text
0x023D8C -> 0x02FFF0 -> 0x020430
                         -> read 0x60C001 / 0xA80001
                         -> update records at 0xFF9000..0xFF9007
                         -> advance 0xFFDB4A
```

This is statically and dynamically supported as an input/state path, but its
fields are not yet proven to be player controls or gameplay state. A separate
late-drive trace establishes the first input-dependent main-CPU divergence at
`0x02046A`/`0x02046C`.

Evidence: `analysis/interconnect.md`,
`reference/experiments/stunrun/input-handler-search.metadata.json`,
`reference/experiments/stunrun/drive-input-rendered-delta.metadata.json`.

### 3. Simulation/update work

The following work is sufficiently bounded to model as per-tick or per-frame
update stages:

- elapsed-update counter at `0xFF9564`;
- time-remaining mechanism at `0xFF9568`;
- speed/velocity candidate at `0xFFDD16`, including a bounded increase and
  clamp path;
- trajectory integration/clamping around `0xFFDCC6` and the trajectory record
  at `0xFFDAF0`;
- six event timers at `0xFFDE8A..0xFFDE94`;
- object/animation records based at `0xFFDD86`, indexed with a 16-byte stride.

The live object-record update path is particularly clear mechanically:

```text
0x03CF40 -> 0x03D120 / 0x03D1A0
          -> update 16-byte record fields
          -> later geometry/display consumer (branch still unresolved)
```

The record is directional/object-animation state, not a proven health,
weapon, score, or timer structure.

Evidence: `analysis/ram-map.md`,
`analysis/object-record-renderer-path.md`,
`analysis/object-record-timing.md`.

### 4. Road and geometry submission

The road path appears buffered rather than directly rasterized by the main
CPU. In the recorded race interval, PC `0x02248E` drains 384 source words
from `0xFF9584..0xFF9882` in bursts, producing 192 lane-level FIFO writes.
The corresponding GSP VRAM changes occur later, which establishes a
producer/consumer boundary:

```text
main road buffer -> 68010/GSP FIFO -> GSP interpretation -> VRAM
```

The exact road record format remains unknown. The GSP side contains literal
`LINE`, `FILL XY`, `FILL L`, and `PIXBLT B,XY` primitives. The visible output
is therefore plausibly produced by a command stream consumed asynchronously
by the GSP, but the primitive-to-game-object assignments remain open.

Evidence: `analysis/m5-visible-output.md`,
`reference/experiments/stunrun/m5-road-buffer-consumer-trace.metadata.json`,
`reference/experiments/stunrun/m5-gsp-instruction-fork-trace.metadata.json`.

### 5. Cross-processor work

The ADSP is not simply called synchronously from the main loop. It executes
its own program and emits `GINT`; the observed title-path contract is:

```text
ADSP GINT (30 observed writes)
  -> main CPU IRQ2 (30 entries)
  -> handler 0x0213FE
  -> clear 0x818060 at 0x021402
  -> RTE at 0x021418
```

The main CPU also consumes length-prefixed ADSP serial/output blocks and
copies their contents to the GSP FIFO. The payload semantics are unresolved,
but the transport boundary is confirmed.

Sound is similarly asynchronous:

```text
68010 write 0x600000 -> JSA command latch -> 6502 reads $280A
6502 response $2A02 -> main IRQ4 -> 68010 response handler
```

Evidence: `analysis/interconnect.md`,
`reference/experiments/stunrun/adsp-special-io-trace.metadata.json`,
`reference/experiments/stunrun/sound-handler-search.metadata.json`.

## Implementation-shaped pseudocode

This is the smallest loop shape currently justified by the evidence:

```c
for (;;) {
    wait_for_or_advance_fixed_tick();       // global interrupt timebase
    global_tick++;

    service_main_interrupts();              // ADSP IRQ2, sound IRQ4, etc.
    input = sample_inputs();                // exact field semantics pending
    update_state_dispatch(input);           // 0xFF9550 family
    update_timers(global_tick);
    update_vehicle_motion(input);           // speed/trajectory slice
    update_object_records(input, global_tick);
    update_road_buffers(global_tick);
    submit_main_to_gsp_commands();
    submit_sound_events();

    // Workers continue independently; their completion/response interrupts
    // are handled on later main-CPU interrupt entries.
    run_or_wait_for_gsp_adsp_sound_work();
}
```

The native implementation should initially use one deterministic fixed-step
iteration per video frame, while retaining an internal interrupt-tick counter
for the observed approximately four-ticks-per-frame timing. That gives us a
stable game loop without pretending that the original's exact interrupt
scheduling has already been reconstructed.

## What is still missing before calling this the game loop

1. The exact main-CPU scheduler entry and the state-dispatch cases for title,
   attract, and active gameplay.
2. A verified transition from the title/road demo into controllable gameplay.
3. The semantic mapping of the input/state records and the physical vehicle
   state fields.
4. The road-buffer producer format and its relationship to course progression.
5. A direct runtime correlation from object records to their geometry payload.

These are investigation targets, not reasons to invent a complete gameplay
model. The loop skeleton is now strong enough to implement a replayable
simulation shell and attach each subsystem behind an evidence-backed boundary.
