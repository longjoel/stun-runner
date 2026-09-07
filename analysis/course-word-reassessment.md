# Reassessment of `0xFF9578`

The earlier temporal-candidate label treated the four-byte snapshot region
beginning at `0xFF9578` as a single course selector. Direct bus tracing and the
static listing show adjacent fields: `0xFF9578` remains a separate
course/score-selection candidate, while the observed nonzero values were in
the neighboring word at `0xFF957A`/low byte `0xFF957B`.

The routine at `0x02B1D0` reads the low control bits from
`:mainpcb:a80001`, compares the derived value with `0xFF957A`, and writes the
new value at `0x02B1FE`. The routine at `0x02BA68` combines the same control
bits with the `BTST #2` result, masks the result to `0..7`, compares it at
`0x02BA9C`, and stores it at `0x02BAB2`. `0x02BAD6` clears both the state word
and its adjacent transition counter at `0xFF957C`.

The transition-bearing direct read trace observed:

| Frame | Reader PC | Value at `0xFF957A` |
| ---: | ---: | ---: |
| 771 | `0x02B1EE` / `0x02B1F8` | `3` |
| 927 | `0x02BA9C` | `6` |
| 1504 | `0x02BA9C` | `7` |

The snapshots show these in the low byte of `FF9578..FF957B`, explaining why
the earlier two-byte probe at `FF9578` returned zero. The values are input-state
encodings, not course numbering, geometry record strides, or ROM-table
indices. The separate `0xFF9578` score/course flag had not produced a nonzero
transition in the earlier scripted runs and was unresolved at that point.

The configuration matrix is also negative: `service_probe` produced no reads
of the field during frames 600–900, and `sw1_all_on` read `0xFF9578` as zero
while changing only the adjacent `0xFF957A` input-state word. See
`reference/experiments/stunrun/course-flag-config-matrix.metadata.json`.

## Static follow-up on the separate word

The fresh 68010 listing shows that `0xFF9578` is not merely an untouched
configuration byte. It is a persistent state/index word with several
executable writers:

| Writer | Mechanism | Evidence-backed implication |
| ---: | --- | --- |
| `0x02B63E` | stores a byte selected from ROM table `0x0473CA`, indexed by `0xFF9B88` | state is initialized from a ROM-driven selector |
| `0x0252EC` | increments the word after the state/score presentation path | state advances during a transition |
| `0x02572C`, `0x02577A`, `0x0257C8`, `0x025814` | stores `0x000F`, `0x000D`, `0x000C`, or `0x0004` and mirrors the value to `0xFF9556` | explicit state branches select different content/configuration |
| `0x0320BC` | increments it while adding an entry from ROM table `0x04AF7E` to the score accumulator | state controls a progression/award sequence |
| `0x0320FA`, `0x032104` | remaps state `8` to `12` and state `11` to `17` under object/event flags | later progression states are normalized explicitly |

The state is also used as an index into ROM tables at `0x047406`, `0x048180`,
and `0x048068` in the `0x0249EC–0x024B36` path. This makes `0xFF9578` a
strong course/mode/progression candidate, but does not by itself prove that
its value is the displayed track number. The current late-drive writer trace
still observed only zero-valued writes at `0x021302`, `0x024322`, `0x02B63E`,
and `0x02B95E`; reaching the nonzero branches requires a new gameplay
transition or a save state taken there.

## Human-play progression capture

The bounded real-time capture `human-course-progression` then observed the
word changing through `0→1→2→3→4→5`. The transitions occurred at frames
`3142`, `7019`, `10377`, `15310`, and `19862`; a short terminal cleanup path
also produced `5→4→5` at frames `20105` and `20108`. Each 600-frame sample
has a matching local save state, and representative snapshot/state hashes are
recorded in `reference/experiments/stunrun/human-course-progression.metadata.json`.

This promotes `0xFF9578` from a static candidate to a dynamically confirmed
progression/state field. It still does not prove that the stored number is
the on-screen track label, nor that the terminal `5→4→5` sequence represents
three distinct courses.

Evidence: `reference/experiments/stunrun/course-word-reader-trace.metadata.json`,
`/tmp/stunrun-course-read-long/result.json`, and the main-CPU listing around
`0x02B1D0` and `0x02BA68`.
