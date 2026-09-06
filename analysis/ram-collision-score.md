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

## Current conclusion

`0xFFDD0C`, `0xFFDD10`, `0xFFDD4E`, and `0xFFDD50` are retained as literal
static candidates. `0xFFDD52–0xFFDD5A` is an observed adjacent event/object
cluster. No field in this experiment is promoted to armor, weapon, or ammo.
The next useful experiment is a controlled collision or damage interaction,
not more passive firing in the current course segment.
