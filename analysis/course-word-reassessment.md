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
indices. The separate `0xFF9578` score/course flag has not produced a nonzero
transition in the current runs and remains unresolved.

Evidence: `reference/experiments/stunrun/course-word-reader-trace.metadata.json`,
`/tmp/stunrun-course-read-long/result.json`, and the main-CPU listing around
`0x02B1D0` and `0x02BA68`.
