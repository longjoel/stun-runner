/* See fifo_block.h for provenance and confidence notes. */
#include "fifo_block.h"

uint32_t stunrun_fifo_terminator_address(uint32_t base, unsigned count)
{
    return base + 2u * count;
}

int stunrun_fifo_block_valid(uint16_t terminator)
{
    return terminator == STUNRUN_FIFO_TERMINATOR;
}

size_t stunrun_fifo_transfer(const uint16_t *payload, unsigned count,
                             uint16_t *fifo_out, size_t fifo_cap)
{
    unsigned i;
    if (payload == NULL || fifo_out == NULL)
        return 0;
    if ((size_t)count > fifo_cap)
        return 0;
    for (i = 0; i < count; i++)
        fifo_out[i] = payload[i];
    return count;
}

static const stunrun_fifo_run_summary_t kObservedRun = {
    STUNRUN_FIFO_RUN_BLOCKS,
    STUNRUN_FIFO_RUN_TERMINATORS,
    STUNRUN_FIFO_RUN_READS,
    1
};

stunrun_fifo_run_summary_t stunrun_fifo_observed_run(void)
{
    return kObservedRun;
}

int stunrun_fifo_run_matches(const stunrun_fifo_run_summary_t *summary)
{
    if (summary == NULL)
        return 0;
    return summary->blocks == STUNRUN_FIFO_RUN_BLOCKS &&
           summary->terminators == STUNRUN_FIFO_RUN_TERMINATORS &&
           summary->reads == STUNRUN_FIFO_RUN_READS &&
           summary->fifo_writes_match_count == 1;
}
