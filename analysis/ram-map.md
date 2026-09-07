# RAM Annotation Map

This is the RAM counterpart to the ROM landmark records. Addresses are kept
literal until a runtime trace, snapshot diff, or MAME map establishes a
stronger claim. A range being listed here does not make it a game-semantic
field.

## Hardware-defined regions

| Address/range | Owner/view | Evidence | Status |
|---|---|---|---|
| `0xFF8000–0xFFFFFF` | 68010 work RAM | pinned `driver_68k_map` | `MAME-CONFIRMED` |
| `0xFF4000–0xFF4FFF` | 68010 ZRAM view combining the M48T02 high byte and 2816 EEPROM low byte | pinned `driver_68k_map` / `hd68k_zram_r/w` | `MAME-CONFIRMED` |
| `0x0000–0x1FFF` (ADSP data space) | ADSP-2100 internal data RAM | runtime ADSP address space and direct snapshots | `MAME-CONFIRMED` |
| `0x0000–0x1FFF` (ADSP program space) | ADSP-2100 program RAM | runtime map, upload taps, program snapshots | `MAME-CONFIRMED` |
| `0x800000–0x807FFF` | 68010 view of ADSP program RAM | `hd68k_adsp_program_r/w` | `MAME-CONFIRMED` |
| `0x808000–0x80BFFF` | 68010 view of ADSP data RAM | `hd68k_adsp_data_r/w` | `MAME-CONFIRMED` |

The ADSP data/program addresses above are separate spaces; `DM($0955)` is not
the same address as a 68010 byte address. The host windows are device views,
not ordinary 68010 work RAM.

## Runtime-annotated locations

| Address/range | Literal observation | Confidence |
|---|---|---|
| `DM($0955–$0956)` | startup writes `0x1242, 0x124E`; the same pair appears in the frame-411 data-space delta | `OBSERVED-IN-TRACE + SNAPSHOT-DIFF` |
| `DM($0959–$095A)` | startup writes `0x7FFF, 0xFFFF`; the same pair appears in the frame-411 data-space delta | `OBSERVED-IN-TRACE + SNAPSHOT-DIFF` |
| `DM($0957–$0958)` | startup probe observes zero values in the reconstructed slice | `OBSERVED-IN-REPLACEMENT` |
| `0xFF9000–0xFF9003` | input-adjacent 68010 RAM record read/writes at static candidate `0x020430`; field meaning unresolved | `STATIC-CANDIDATE + OBSERVED-IN-TRACE` |
| `0xFF9004–0xFF9007` | adjacent sampled record; no late-control differential in the tested schedules | `OBSERVED-NEGATIVE` |
| `0xFFDB4A` | byte advanced by the static input candidate; late-control landmark sample | `STATIC-CANDIDATE` |
| `0xFFDAEE` | sampled update flag; no late-control differential in the tested schedules | `OBSERVED-NEGATIVE` |
| `0xFF948C–0xFF948E` | input-dependent counter/accumulator neighborhood used by `0x0418B8`; late-drive adds writes from `0x041956` and `0x04195A` | `STATIC + OBSERVED-IN-TRACE` |
| `0xFFDBA4` | one byte in an indexed table written by `0x030212` from table base `0xFFDB64`; semantic ownership unresolved | `STATIC + OBSERVED-IN-TRACE` |
| `0xFF9564–0xFF9567` | elapsed-update counter candidate; `0x024506` increments the longword continuously during the gameplay window | `STATIC + OBSERVED-IN-TRACE` |
| `0xFF9568–0xFF956B` | displayed time-remaining mechanism; initialized from ROM, decremented by timer logic, tested for expiry and a `0x988` threshold, and combined with a track-indexed base by HUD routine `0x028CA8` | `DYNAMIC-TIME-REMAINING-CONFIRMED` |
| `0xFF9578–0xFF9579` | separate course/score-selection flag read by the object-award path at `0x03A2B2`, `0x03A2DA`, and `0x03A2F8`; all current runs leave it zero | `STATIC-CANDIDATE; NONZERO TRANSITION UNREPRODUCED` |
| `0xFF957A–0xFF957B` | input-state word derived from `:mainpcb:a80001` and masked to control-state values; adjacent transition counter at `0xFF957C` | `DYNAMIC-INPUT-STATE-CONFIRMED` |
| `0xFF9532–0xFF9535` | live score accumulator; cleared at several game-state entries, formatted/displayed by nearby UI paths, and observed receiving monotonic 50-point increments at `0xFF9534` from PC `0x03A2EC` | `DYNAMIC-SCORE-CONFIRMED` |
| `0xFF9544` | object/event attribute flag; static paths derive it from bit 7 of active object-record fields at `0x0323A4`/`0x03244A`, rather than from a persistent craft-status record | `STATIC-RESOLVED-OBJECT-FLAG` |
| `0xFFDE8A–0xFFDE94` | six 16-bit event-slot timers; `0x0416D8` reloads a slot from ROM table `0x4EF20`, `0x041644` decrements it, and zero may invoke the event/effect path at `0x041606` when `0xFF9550 == 1` | `STATIC + EVENT-TRACE; NOT-HEALTH` |
| `0xFFDE9A–0xFFDEBA` | 32-byte object-event flag table; `0x03A084` sets indexed bytes for object-record bit `0x4000`, while `0x04171C` clears the table | `STATIC + EVENT-TRACE; NOT-HEALTH` |
| `0xFF4410–0xFF44FF` | ten-entry persistent high-score table in the mapped ZRAM view; 24-byte records contain a big-endian score at `+0` and a display name beginning at `+2` | `SNAPSHOT + PERSISTENT-NVRAM + STATIC` |
| `0xFFDD16–0xFFDD17` | speed/velocity candidate; reset to zero, increased in `0x02820A` by `0x20`, bounded at `0x3C0`/`0x500`, displayed through the nearby HUD path, and consumed by motion math at `0x039E04` | `STATIC + OBSERVED-IN-TRACE` |
| `0xFFDD02` | object-hit/progression counter; `0x03A298` increments it after the object collision check, and the following `0x03A2EC` path awards `50` or `500` points based on `0xFF9578` | `DYNAMIC-OBJECT-HIT-COUNTER-CONFIRMED` |
| `0xFFDD04`, `0xFFDD06`, `0xFFDD08` | coordinate/object-state candidates passed through collision/bounds helpers and copied into historical comparison fields `0xFF9576`, `0xFF9574`, `0xFF9570`; exact ownership remains unresolved | `STATIC-CANDIDATE + OBSERVED-IN-TRACE` |
| `0xFFDD1A–0xFFDD26` | active movement/physics cluster updated by the drive path; exact axis, steering, acceleration, and renderer roles remain unresolved | `OBSERVED-IN-TRACE` |
| `0xFFDD86–0xFFDDA5` | base of a 16-byte indexed object/animation record array; `0x03D012` indexes records by `0x10`, while `0x03D18C`, `0x03D1B2`, `0x03D208`, and `0x03D248` update interpolation fields; lateral forks expose this renderer/object state | `STATIC-RESOLVED-OBJECT-ANIMATION; NOT-HEALTH-OR-AMMO` |
| `0xFFDD0C` | static collision/scoring path accumulates a count-derived contribution here before adding it to the live score; runtime meaning and event ownership remain unresolved | `STATIC-CANDIDATE` |
| `0xFFDD10` | static collision/scoring path initializes a short event cooldown to `0x14`; the aligned scoring-window trace did not observe a write to this location | `STATIC-CANDIDATE + OBSERVED-NEGATIVE` |
| `0xFFDD4E` | static object-event path compares this byte with `0x32` and later reloads `0x3C`; snapshots show it changing during the scoring window, but the tap did not isolate its writer | `STATIC-CANDIDATE + SNAPSHOT-OBSERVED` |
| `0xFFDD50` | static object-event path increments this field before deriving a `0xC8` score contribution; it remained zero in the bounded weapon/scoring snapshots, so collision-event ownership is unconfirmed | `STATIC-CANDIDATE + OBSERVED-NEGATIVE` |
| `0xFFDD52–0xFFDD5A` | adjacent event/object state words with repeated writers during the scoring window; individual meanings are unresolved | `WRITER-TRACE-OBSERVED` |
| `0xFFDCC0–0xFFDCE0` | transient object/effect trajectory state; `0x0387C6` initializes the template, `0x03887A` integrates it using `0xFFDD16`, and writers `0x0388DE–0x038A14` produce a coherent 17-pass record after the paired Button 1 → Button 2 sequence | `STATIC + WRITER-TRACE + PAIRED-INPUT-DIFFERENTIAL` |
| `0xFF9E96–0xFF9E99` | paired-input ROM effect/object descriptor pointer; PC `0x02803A` writes `0x0004A004` (ID `0xD2` at `0x048F9C + id*0x14`) before calling the trajectory-timer reset at `0x02804A` | `DYNAMIC + MAME-LISTING-RESOLVED` |
| `0xFF9582` | 5-bit cyclic animation/state counter; `0x0268EC` increments it and masks with `0x1F`, while UI/object paths read it | `STATIC-RESOLVED-ANIMATION-COUNTER` |
| `0x80BFFE` | one observed 68010 write of `0xFFFF` during bounded title-path initialization | `OBSERVED-IN-TRACE` |

## ROM-to-RAM mechanism annotations

These entries are intentionally literal descriptions of the disassembled
instructions and observed writes:

| RAM location | ROM mechanism | Confidence |
|---|---|---|
| `0xFF9000–0xFF9003` | routine `0x020430` derives four byte positions from input/status reads; `0x020494` and `0x020498` store the derived bytes | `STATIC + OBSERVED-IN-TRACE` |
| `0xFF9006` | `0x02048C` increments the byte at offset `+6` from the `0xFF9000` base while processing a nonzero input byte | `STATIC + OBSERVED-IN-TRACE` |
| `0xFF948F` | `0x0204B8` increments this byte after the `0x0418B8` helper returns | `STATIC + OBSERVED-IN-TRACE` |
| `0xFF9490` | `0x020510` increments this byte before calling `0x02053C`; `0x020544` increments it again inside that routine | `STATIC + OBSERVED-IN-TRACE` |
| `0xFF9498` | `0x02053C` uses this as a base pointer for a state/cache structure; offsets `+0x16`, `+0x17`, `+0x19`, `+0x1C–0x21`, `+0x28`, and `+0x2A` are accessed | `STATIC-CANDIDATE` |
| `0xFF94AE–0xFF94BE` | the `0x0205E2–0x02061E` subpath stores source bytes and derived XOR bytes at offsets from the `0xFF9498` structure base; the late-drive trace adds these writers | `STATIC + OBSERVED-IN-TRACE` |
| `0xFF9498–0xFF94A4` | late-drive-only initialization writes occur at `0x020680`, `0x0206BE`, `0x0206C8`, and `0x020CA6`; observed values include `0x0404`, `0x0808`, `0x12FF`, and `0xFF2E` | `OBSERVED-IN-TRACE` |
| `0xFF94AE` | repeated high-byte writes from the `0x020586`/`0x020634`/`0x02063A` path form a descending pattern beginning `0x0F`, `0x0E`, `0x0D`, …, `0x05` in the captured window | `OBSERVED-IN-TRACE` |
| `0xFF94B0–0xFF94BE` | one captured derived-byte pass writes `0xFE`, `0x01`, then zero-valued low/high-byte components under the masks recorded by the tap | `OBSERVED-IN-TRACE` |
| `0xFF94C0` | periodic `0x0206CC` writes are `0x0010` in no-input; the late-drive sequence begins `0x0010`, `0xFF1A`, `0x421D`, `0x421F`, `0x4221`, … | `OBSERVED-IN-TRACE` |
| `0xFF94C2–0xFF94C5` | `0x0206D0` writes the long pointer at structure offset `+0x2A`; no-input remains zero, while late-drive points into the derived-byte area beginning at `0xFF94B1` | `STATIC + OBSERVED-IN-TRACE` |

The structure and counters are not yet assigned game meanings. In particular,
the repeated `0x0205E2` stores may be a decoded/cache or renderer-support
operation; the current evidence does not justify calling them player fields.

The three-way snapshot campaign (`none`, coin/start-only `late`, and
coin/start plus steering/button `late_drive`) is recorded in
`reference/experiments/stunrun/main-ram-candidate-ranking.metadata.json`.
It separates raw input plumbing at `0xFF8000–0xFF8004` from persistent
coin/start-dependent candidates, but does not identify score, timer, track,
craft, armor, or weapon semantics. Those labels require controlled gameplay
events and writer attribution for each candidate.

The adjacent pointer trace confirms the structure layout at runtime:
`0x0206CC` writes `A1` to `+0x28` (`0xFF94C0`), and `0x0206D0` writes `A0`
to `+0x2A` (`0xFF94C2–0xFF94C5`). See
`reference/experiments/stunrun/main-ram-pointer-trace.metadata.json`.

Temporal correlation provides the first high-value gameplay-state candidates.
`0xFF9568` is initialized from ROM at `0x024372`, decremented through the
`0x02907E–0x029132` timer/expiry path, and checked against `0x988` at
`0x032F78`; its observed value falls from `8540` to `7966` in the bounded
late-drive run. The four-byte snapshot region at `0xFF9578` was initially
mistaken for one course index because its low byte takes values `0`, `3`, and
`6` in temporal reports. Direct tracing separates the adjacent fields:
`0xFF957A` is the input-state word, while `0xFF9578` remains a distinct word
tested by the score-award path. `0x02B1EE`/`0x02B1F8` derive and compare
`0xFF957A` against the digital control port; `0x02BA9C` derives a second `0..7`
control state including the `BTST #2` input and stores it through `0x02BAB2`.
The `0xFF957C` word counts state transitions. The course meaning of
`0xFF9578` remains open.
The timer field is now promoted as the displayed time-remaining mechanism:
`0x028CA8` combines it with the current course index and formats the result
through the HUD path, while `0x02907E–0x02910A` performs expiry handling and
score awards. Its displayed unit is still unresolved.
Full provenance is in
`reference/experiments/stunrun/main-ram-temporal-candidates.metadata.json`.

The adjacent `0xFF9532` longword is the live score accumulator. Static
listing evidence shows clears at `0x024334`, `0x024BE6`, `0x027196`, and
`0x02B624`, display/formatting reads at `0x024CEA`, `0x024DA6`, `0x026A60`,
and `0x042168`, and additions at `0x028DFA`, `0x028E54`, `0x028F1C`, and the
timer path `0x0290A8–0x02910A`. A controlled weapon-input run then observed
ten monotonic 50-point writes at `0xFF9534` from PC `0x03A2EC`, while the high
word at `0xFF9532` remained zero. See
`reference/experiments/stunrun/main-ram-score-candidate.metadata.json`.

The persistent high-score table is now identified. The ZRAM view at
`0xFF4410–0xFF44FF` contains ten 24-byte records. Snapshot decoding gives
scores `15000, 12500, 10000, 8000, 7000, 5000, 4000, 3000, 2000, 1000` and
the names `THE GONZ`, `SMOKY`, `GUNNER GLENN`, `BAD BABE`, `THE POTATOE`,
`SCOOTER`, `THE HOOPLE`, `RANGER RICK`, `BUGS`, and `POGO`. The table is
backed by the two persistent byte lanes `:mainpcb:200e` (M48T02) and
`:mainpcb:210e` (2816 EEPROM); neither lane should be treated as ordinary
work RAM. See
`reference/experiments/stunrun/main-nvram-high-score-table.metadata.json`.

The drive/object cluster is now split: `0xFFDD02` is an object-hit/progression
counter, while `0xFFDD04/06/08` remain bounded coordinate/object-state
candidates whose historical values are stored at `0xFF9576/74/70`.
The neighboring `0xFFDD16–0xFFDD26` fields form the active motion cluster.
`0xFFDD16` is stronger: it is reset, stepped by `0x20`, bounded at `0x3C0`
and `0x500`, passed to a HUD formatting path, and used in motion calculations.
The synchronized late versus late-drive snapshots diverge first around the
steering event, and the late-drive writer trace records repeated updates to
`0xFFDD16`, `0xFFDD1A–0xFFDD26`, and the coordinate candidates. These are
craft-motion candidates, not armor or weapon labels. A subsequent extended
object/flag trace found only renderer/object traffic and transient
`0xFFDC1C/0xFFDC1E` updates under the repeated-fire schedule; it did not
isolate a damage, armor, weapon, or ammunition transition. Provenance is in
`reference/experiments/stunrun/main-ram-craft-state-candidates.metadata.json`.
The negative control and extended tap are recorded in
`reference/experiments/stunrun/main-ram-weapon-probe.metadata.json`.

A structure-only trace confirms the separation: the no-input run records only
84 periodic `0x0206CC → 0xFF94C0` writes, while late-drive records 122 writes
and enters the `0x020586–0x0206C8` path at frame 681. The first new store is
`0x020CA6 → 0xFF949A`; the repeated derived-byte stores cover
`0xFF94AE–0xFF94BE`. See
`reference/experiments/stunrun/main-ram-structure-trace.metadata.json`.

## Snapshot-diff clusters

The first full-RAM input comparison used identical clean boots and captured
the 68010 work-RAM range at frames 600, 650, 700, 750, 800, 850, and 900.
There are no differences at frames 600 or 650. The first divergence is 61
bytes at frame 700, after the Coin 1/Start events; it is 27 bytes at frame
750, then expands to 1,875 bytes at frame 800 after the steering/Button 1
event and reaches 3,163 bytes at frame 900. Large frame-900 clusters include:

```text
0xFF95E3–0xFF970A   296 bytes
0xFF98E3–0xFF9A0A   296 bytes
0xFF97A0–0xFF9850   177 bytes
0xFF9AA0–0xFF9B50   177 bytes
0xFF9718–0xFF979E   135 bytes
0xFF9A18–0xFF9A9E   135 bytes
```

These are candidate state/working regions only. The snapshot diff establishes
coincident change, not ownership or meaning. Provenance is recorded in
`reference/experiments/stunrun/main-ram-snapshot-diff.metadata.json`.

## Writer-PC attribution

A bounded write trace over `0xFF9000–0xFF9FFF` from frames 680–705 narrows the
cause of the first divergence. The first event mismatch occurs at frame 680;
the first writer present only in the late-drive run is `0x02048C` at frame 681,
writing `0xFF9006`. A concentrated follow-on path in `0x0205E2–0x02063A`
writes `0xFF94AE–0xFF94BE`, with `0x0205E2` as the repeated writer. These are
the first RAM annotations tied to input-differential writer PCs rather than
only to final snapshot contents. See
`reference/experiments/stunrun/main-ram-write-trace-differential.metadata.json`.

## Open annotation work

- correlate changed RAM clusters with writer PCs and exact input events;
- split persistent state from renderer/temporary buffers;
- add normalized field annotations only after repeated cross-run evidence;
- keep host device windows separate from ordinary 68010 RAM in selectors.
