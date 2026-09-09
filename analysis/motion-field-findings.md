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

The next discriminating experiment is a same-state schedule pair that changes
only steering and captures this field plus `0xFFDCC0`, the trajectory record,
and the road-buffer/FIFO boundary in one run. Full provenance for this probe is
in
`reference/experiments/stunrun/m5-motion-field-write-trace.metadata.json`.
