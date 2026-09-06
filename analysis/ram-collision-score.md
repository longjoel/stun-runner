# Collision and score-adjacent RAM probe

This note records a bounded attempt to connect the static object-event path
with runtime RAM state. It does not promote the fields below to player armor,
weapon, or ammunition semantics.

## Static mechanism

The main-CPU listing shows an object-event path around `0x03A000` that, after
the collision/object check succeeds, performs the following literal actions:

- tests `0xFFDD4E` against `0x32`;
- when `0xFFDCF2` is clear, increments `0xFFDD50`;
- derives `0xFFDD0C` from that count using a `0xC8` multiplier;
- adds the contribution to the live score at `0xFF9532`;
- increments `0xFFDD08`, sets a bit in `0xFF9EA8`, and sets `0xFFDC1C`;
- reloads `0xFFDD4E` with `0x3C` and, on the alternate branch, initializes
  `0xFFDD10` with `0x14`.

This is strong evidence for an event/scoring bookkeeping mechanism, but not
for the identity of the affected object or the player's damage state.

## Runtime probes

The first probe used `late_drive`, device `:mainpcb:maincpu`, range
`0xFFDD00–0xFFDD5F`, and snapshots at frames 600, 900, 1200, 1500, and 1800.
Result log hash:

```text
a08b8a604279821ebec7522091d7c115bc3de43eca196b4e558deaf2b13c6569
```

The movement/object cluster becomes populated by frame 1800. The suspected
event fields `0xFFDD0C`, `0xFFDD10`, and `0xFFDD50` remain zero in that run.

The second probe used `weapon_probe`, the same RAM range, frames 3000–3400,
and the known score-writing window around frames 3277–3361. Its snapshot
campaign result hash is:

```text
3b2520bba2ece2a5184be5e6454b0e6d046c063f1fbadce5575618d7c54ed631
```

Snapshots show `0xFFDD4E` changing while the score advances and the coordinate
record at `0xFFDD04/06` changes. `0xFFDD0C`, `0xFFDD10`, and `0xFFDD50` remain
zero at frames 3000, 3270, 3280, 3300, 3340, and 3400.

The corrected main-CPU write tap captured 2,341 events in the same window;
canonical log hash:

```text
7fcd07f247fa0dc177c23f445962964d95c941c238e610b4b76b4b025ea5bbf8267
```

The most active adjacent fields were `0xFFDD10`, `0xFFDD12`, `0xFFDD14`,
`0xFFDD16`, `0xFFDD18`, `0xFFDD1C`, `0xFFDD1E`, `0xFFDD20`, `0xFFDD22`,
`0xFFDD24`, `0xFFDD26`, `0xFFDD52`, `0xFFDD54`, `0xFFDD56`, and `0xFFDD58`.
The trace confirms an active object/physics record but does not isolate an
armor, weapon, or ammunition transition.

## Steering differential control

To separate a putative collision from ordinary steering, the same start and
Button 1 schedule was rerun with the analog X value set to `0` at frame 900
instead of `220`; both schedules returned to centered input at frame 1500.
The reusable schedule is `experiments/stunrun/collision_probe_left.json` and
the snapshot mode is `collision_probe_left`.

In `0xFFDD00–0xFFDD5F`, the two runs are identical at frames 900, 1200, and
1500. At frame 1800 only `0xFFDD24`, `0xFFDD26`, and `0xFFDD4E` differ. In the
status window `0xFF9500–0xFF95FF`, the runs are identical through frame 1500;
at frame 1800 only seven bytes differ: `0xFF9531`, `0xFF9544`, `0xFF9567`,
`0xFF956B`, `0xFF957B`, `0xFF957D`, and `0xFF9582`. The confirmed score
(`0xFF9532–0xFF9535`), time (`0xFF9568–0xFF956B` as a word mechanism), and
track candidate (`0xFF9578`) do not show a new steering-specific transition.

This is a steering/physics control result, not a collision or damage result.
It narrows the next experiment: a damage probe must place the craft in a
different interaction state, rather than simply changing X input during this
road segment.

## Save-state fork at a live checkpoint

The reusable save-state path was exercised from a live `late_drive` checkpoint
captured at relative setup frame 2400. A no-input fork and the
`fork_lateral_sweep` input fork both advanced beyond the loaded state, proving
that the checkpoint can be used without replaying coin/start/setup. The status
window `0xFF9500–0xFF95FF` produced no value differential between the two
forks at relative frames 2, 300, 600, 1200, and 1800; in particular, this
control did not isolate armor, weapon, ammunition, score, time, or track state.

The craft/object window `0xFFDD00–0xFFDE00` did produce persistent
input-dependent differences. The strongest cluster was `0xFFDD85–0xFFDD90`,
first differing at relative frame 300 and remaining different through frame
1800. The motion field `0xFFDD16` and nearby movement fields also differed at
the later sample. These results promote the cluster as a useful fork-sensitive
object/renderer candidate, not as a health or weapon field. Full hashes and
the exact relative input schedule are recorded in
`reference/experiments/stunrun/main-ram-live-fork-differential.metadata.json`.

The checkpoint was then forked into sustained-left and sustained-right
controls for 3600 relative frames, with Button 1 and Button 2 explicitly
released at the fork boundary. The entire `0xFF9500–0xFF95FF` status window
was byte-identical between the extremes at relative frames 600, 1200, 1800,
2400, 3000, and 3600. Score, timer, track, event flags, and the known
animation counter therefore show no lateral-extreme-specific transition in
this segment. The `0xFFDD00–0xFFDE00` craft/object window does distinguish the
branches, most strongly at `0xFFDD85–0xFFDD90`, while the status window remains
unchanged. This is stronger evidence for a motion/object/renderer record than
for armor or weapon state. The schedules and hashes are preserved in
`reference/experiments/stunrun/main-ram-live-fork-extremes.metadata.json`.

Finally, a full-work-RAM scan (`0xFF8000–0xFFFFFFFF`) compared a centered
fork against the Button 2 sweep from the same center-run checkpoint. The
highest persistent differences were the raw input mirrors at `0xFF8000` and
`0xFF8004`, the already-known `0xFFDD86–0xFFDD8A` object/renderer cluster, and
short-lived high-address clusters around `0xFFFBED–0xFFFC19` and
`0xFFFC12–0xFFFDA1`. No new persistent candidate in the status region or a
weapon/ammunition semantic field was isolated. The full scan is retained as
`reference/experiments/stunrun/main-ram-full-button2-differential.metadata.json`.

## Longitudinal status control

The weapon schedule and a matching `late_drive` control were extended to
frame 6000 with status snapshots at frames 1800, 3000, 3270, 3300, 3400,
4000, 5000, and 6000. The weapon schedule reaches the already confirmed
50-point scoring sequence (`0xFF9534` reaches `0x01F4` by frame 3400), while
the control does not. The confirmed track word `0xFF9578` remains zero in
both runs at all sampled points.

The large differences beginning at frame 4000 are concentrated in the
adjacent display/progression area, not a newly isolated craft-status byte.
Static listing inspection resolves two tempting bytes: `0xFF9544` is copied
from object-record attribute bit 7 by `0x0323A4`/`0x03244A`, and `0xFF9582` is
the five-bit cyclic counter updated at `0x0268EC`. Neither is armor or health.

## Center-run object-hit confirmation

The no-fire center schedule is a stronger interaction control than the earlier
steering probes. Its object-window trace covers frames 2200–3600 and has
canonical log hash:

```text
b780e4930a2c7fd0a6c3903108c580d5303734fedc1c1264d27e2350e5f3d567
```

The trace records `0xFFDD02` advancing from `1` through `0x22` at PC
`0x03A298`. The corresponding score trace has canonical log hash:

```text
b8c02691475f672e363497229f5334f580f0076270b0628d512e5ca847f8f77a
```

It records PC `0x03A2EC` writing successive 50-point increments at frames
2565–2628, 2925–2943, 3043–3201, 3387–3423, and 3585–3594. The static listing
shows the causal sequence: a successful `0x03B02C` object check reaches
`0x03A298`, increments `0xFFDD02`, selects the track-dependent point value,
and adds it to `0xFF9532`. This confirms `0xFFDD02` as an object-hit or
progression counter and `0xFF9532–0xFF9535` as the score destination in the
same live interaction.

The center run still leaves `0xFFDD0C`, `0xFFDD10`, and `0xFFDD50` at zero;
those remain candidates for a different collision/object class. Nothing in
this successful object-hit path is evidence of craft armor or health.

## Object-event slot control

The same center run was traced over `0xFFDE00–0xFFDEB0` for frames 2200–3600.
The canonical log hash is:

```text
cb8d846869915ac424a3aeabbef648f6aaad92aab93c506ffd94e30a7d454dc6
```

It captured 258 writes. Static inspection resolves six 16-bit words at
`0xFFDE8A–0xFFDE94`; they are repeatedly written by `0x0416D8` from the fixed
table at `0x4EF20` and decremented by `0x041644`. When a slot reaches zero,
`0x041606` consumes the associated table entry and updates event/effect state.
The byte table
`0xFFDE9A–0xFFDEBA` is initialized/cleared by `0x04171C` and receives indexed
ones from `0x03A084` when an object record carries bit `0x4000` at offset
`0x18`.

The focused center-run snapshot campaign gives the six words the values
`20, 15, 20, 1, 1, 1` at frame 2400. The first word changes to `6` by frame
2700 and `0` by frame 2800, while the other five remain unchanged. A follow-up
write trace resolves that transition as timer activity: repeated reloads at
`0x0416D8` write `20`, then `0x041644` decrements the first slot once per
roughly three frames. The event consumer at `0x041606` first requires
`0xFF9550 == 1`; that state word stayed zero in the sampled run, and the
`0xFF954E/0xFF9550` effect bytes did not change at depletion. These six words
are therefore event timers, not a confirmed shield or health store.

The generated listing also shows `0x041606` called from multiple object/effect
handlers, including `0x032ADE`, `0x03A14E`, `0x03A36A`, and `0x03A668`, with
different small indices. This shared call pattern further rules out treating
the timer bank as a six-item player inventory. The actual shield/armor search
must follow a damage-specific object branch and its player-state writes.

## Sustained steering negative control

The centered no-fire schedule was repeated with `AD Stick X=0` held from
frame 900 through frame 3600 (`experiments/stunrun/collision_probe_hold_left.json`).
The status snapshots show no score writes through frame 3600, while the timer
and surrounding progression state continue. The event table remains quiet
until indexed flags appear at `0xFFDE9A` and `0xFFDE9C` around frames 3000–3270;
these are the same object-event table mechanisms above, not a new craft-status
transition.

This control does not demonstrate a wall-crash or damage state. It does,
however, distinguish the center-run object-hit/score path from sustained
steering and narrows the remaining health search to object-record types and
flag branches not reached by these schedules.

The symmetric right-held control (`experiments/stunrun/collision_probe_hold_right.json`)
also produces no score, track change, or sampled effect-state transition
through frame 3600. This rules out a simple left/right wall interaction in
the tube segment; the next damage probe must target an identifiable moving
object or later non-tube segment.

The existing weapon schedule was then extended with the snapshot harness's
unthrottled mode through frame 9000. It reaches `0xFF9534 = 0x0438` (1080
points) by frame 9000, with additional status/display differences appearing
after frame 7500, but `0xFF954E`, `0xFF9550`, and `0xFF9578` remain zero at all
sampled endpoints. This is a longer negative control for the score/effect
cluster, not evidence of armor or weapon inventory.

A new `damage_probe_sweep` then held Button 1 while alternating AD Stick X
between `0`, `255`, and center through frame 12000. The status and craft
snapshots show ordinary movement/progression changes, including a transient
zeroed movement block around frame 6000, but no `0xFF954E/0xFF9550` effect
transition, no track-word change, and no isolated persistent damage field.
The sweep is useful as a reproducible stress control; it still did not create
the required enemy-impact interaction.

Static inspection exposed another tempting candidate: the object branch at
`0x03A31E` tests `0xFFDD80 < 6`, increments it after a successful object
check, and records the last object pointer. A direct write trace over the
weapon schedule (frames 600–9000) captured five writes to `0xFFDD80`, all
zeroing writes from `0x02B9E0`, `0x02719C`, or `0x024BE0`; no increment ever
occurred. This capped counter is therefore not an observed shield transition
and remains an unexercised object-class candidate.

The save-state fork was then used to apply `fork_lateral_sweep` from a saved
center-run checkpoint without replaying setup. The branch remained
deterministic and produced no persistent damage transition: `0xFF954E` was
nonzero only in the first fork sample, consistent with a load/state-boundary
artifact, and returned to zero thereafter; `0xFF9550`, `0xFF9578`, and the
score remained unchanged. The checkpoint is valid as a fork-control fixture,
but not yet a confirmed active enemy-impact frame.

## Current conclusion

`0xFFDD0C`, `0xFFDD10`, `0xFFDD4E`, and `0xFFDD50` are retained as literal
static candidates. `0xFFDD52–0xFFDD5A` is an observed adjacent event/object
cluster. No field in this experiment is promoted to armor, weapon, or ammo.
The next useful experiment is a controlled collision or damage interaction,
not more passive firing in the current course segment.
