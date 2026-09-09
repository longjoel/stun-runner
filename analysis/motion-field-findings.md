# Candidate motion-field findings

This note records a bounded field probe, not a final gameplay map. It was
motivated by the native loop scaffold's need for an evidence-backed motion
state, but the trace does not justify assigning a field name yet.

## Observed active-run behavior

The main CPU write trace covered `0xFFDD10–0xFFDD30` during frames 600–1800
of the recorded `late_drive` schedule. It captured 572 events without
truncation. The most useful candidate, `0xFFDD16`, received 49 writes from
frames 767–1797. Its values include `0x003A`, `0x0074`, `0x015C`, `0x0244`,
`0x0353`, `0x0401`, `0x04A7`, and `0x04D7`; the first and later values show a
real active-run progression rather than a one-time initialized constant.

The neighboring words do not behave as one simple packed scalar:

| Address | Writes | Observed writer-PC count | Safe current label |
| --- | ---: | ---: | --- |
| `0xFFDD16` | 49 | 5 | active candidate, semantics unknown |
| `0xFFDD18` | 43 | 2 | neighboring derived field, semantics unknown |
| `0xFFDD1C` | 80 | 8 | neighboring derived/record field, semantics unknown |
| `0xFFDD20` | 47 | 3 | counter/record candidate, semantics unknown |

The saved-state center and left forks from `/tmp/stunrun-latedrive2400.sta`
each captured 52 initialization/clear events and no later update in the
window. That is a useful control for the chosen checkpoint interval, but it
does not prove that the field is stationary under all no-steering conditions.

## Interpretation boundary

The active-run progression makes `0xFFDD16` worth tracing alongside trajectory
and GSP submissions. It does **not** prove “speed”: the field has multiple
writers, the recorded input schedule changes more than steering, and its
relationship to `0xFFDCC0` has not been directly established. In particular,
the native scaffold must not replace the observed trajectory transform with
`0xFFDCC0 = global_tick`, nor derive speed as “one unit per nonzero steering.”
Those are implementation placeholders, not oracle facts.

## Static arithmetic boundary

The regenerated main-CPU listing gives the literal update shape around the
active writer at `0x03ABDE`:

```text
if (timer_at_FFDD4E != 0 && FFDD16 < 0x0B00)
    FFDD16 += 0x40;
else if (FFDD16 > (local_limit + 0x40))
    FFDD16 -= 0x20;
else
    FFDD16 = FFDD1E + FFDD1A;
FFDBFE += FFDD16;
```

The pseudo-code preserves only the observed operations; the local limit and
timer meaning are not assigned here. Initialization at `0x03B18C` and
`0x03B29E` clears `0xFFDD16`, while neighboring fields are initialized by
separate stores. This is enough to replace the native “increment on steering”
placeholder with a future literal slice once its caller inputs are traced.

The containing routine begins at `0x039B82`. Its setup assigns the local word
at `-0x18(A6)` the literal `0x07E0` before entering the update path, so the
comparison at `0x03ABB4–0x03ABC0` is against a caller-frame limit plus
`0x40`, not directly against an input byte. The same routine keeps separate
arguments at `($0A,A6)` and `($0F,A6)` and uses a pointer at `-0x0A(A6)` for
record flags. Those inputs influence several branches, but their gameplay
semantics are not established by this listing alone.

The listing also resolves the apparent `0xFFDB38` steering differential:
`0x02F246` executes `move.l $FFFF8014, $FFDB36`, so `0xFFDB38` is the low
half of a copied global tick in this path. Its one-unit fork difference is
therefore not evidence of a physical track cursor.

Static provenance is the pinned listing
`/tmp/stunrun-motion-listing/maincpu-68010.lst`, SHA-256
`171e56ccf1a9940635a2e8e9929ab7edaa5841ac3238ef78acf0d6630f8e45e7`.

The same-state center/left pair is now recorded separately in
`reference/experiments/stunrun/m5-same-state-motion-differential.metadata.json`.
The next discriminating experiment is to trace the caller inputs at the
`0x03AB` update block while pairing its field writes with the road-buffer/FIFO
boundary. That can establish which local limit and timer inputs drive the
literal arithmetic without assigning a gameplay name prematurely. Full
provenance for the active-run probe is in
`reference/experiments/stunrun/m5-motion-field-write-trace.metadata.json`.
