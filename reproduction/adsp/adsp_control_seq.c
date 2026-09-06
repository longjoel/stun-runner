/* See adsp_control_seq.h for provenance and confidence notes. */
#include "adsp_control_seq.h"

/* Exact table from adsp-control-window.metadata.json control_map. */
static const stunrun_adsp_control_op_t kOps[] = {
    { 0x818002u, 1, "LEDs", 54u },
    { 0x818008u, 4, "default/control", 2u },
    { 0x81800Au, 5, "/BR assert", 2u },
    { 0x81800Cu, 6, "/HALT assert", 1u },
    { 0x81800Eu, 7, "ADSP reset assert", 3u },
    { 0x818014u, 10, "default/control", 2u },
    { 0x818016u, 11, "deferred ADSP bank switch", 1u },
    { 0x818018u, 12, "default/control", 2u },
    { 0x81801Au, 13, "/BR release", 2u },
    { 0x81801Cu, 14, "/HALT release", 2u },
    { 0x81801Eu, 15, "ADSP reset release", 2u },
};

static const uint32_t kIrqClearPcs[] = { 0x021426u, 0x02C234u };

size_t stunrun_adsp_control_op_count(void)
{
    return STUNRUN_ADSP_CONTROL_OP_COUNT;
}

const stunrun_adsp_control_op_t *stunrun_adsp_control_ops(void)
{
    return kOps;
}

int stunrun_adsp_control_lookup(uint32_t address)
{
    size_t i;
    for (i = 0; i < STUNRUN_ADSP_CONTROL_OP_COUNT; i++) {
        if (kOps[i].address == address)
            return (int)i;
    }
    return -1;
}

const uint32_t *stunrun_adsp_irq_clear_pcs(void)
{
    return kIrqClearPcs;
}

int stunrun_adsp_irq_clear_pc_known(uint32_t pc)
{
    size_t i;
    for (i = 0; i < STUNRUN_ADSP_IRQ_CLEAR_PC_COUNT; i++) {
        if (kIrqClearPcs[i] == pc)
            return 1;
    }
    return 0;
}

stunrun_adsp_upload_state_t stunrun_adsp_upload_state_at(unsigned frame)
{
    if (frame <= STUNRUN_ADSP_UPLOAD_LAST_ZERO_FRAME)
        return STUNRUN_ADSP_UPLOAD_EMPTY;
    if (frame < STUNRUN_ADSP_UPLOAD_COMPLETE_FRAME)
        return STUNRUN_ADSP_UPLOAD_POPULATING;
    return STUNRUN_ADSP_UPLOAD_COMPLETE;
}

int stunrun_adsp_install_frame_ready(unsigned frame)
{
    return frame >= STUNRUN_ADSP_REPLACEMENT_INSTALL_FRAME;
}

void stunrun_adsp_control_tally_init(stunrun_adsp_control_tally_t *tally)
{
    size_t i;
    if (tally == NULL)
        return;
    for (i = 0; i < STUNRUN_ADSP_CONTROL_OP_COUNT; i++)
        tally->per_op[i] = 0;
    tally->unknown_addresses = 0;
    tally->data_mismatches = 0;
}

int stunrun_adsp_control_tally_add(stunrun_adsp_control_tally_t *tally,
                                   uint32_t address, unsigned data)
{
    int index;
    if (tally == NULL)
        return -1;
    index = stunrun_adsp_control_lookup(address);
    if (index < 0) {
        tally->unknown_addresses++;
        return -1;
    }
    tally->per_op[index]++;
    if (data != STUNRUN_ADSP_CONTROL_DATA)
        tally->data_mismatches++;
    return index;
}

int stunrun_adsp_control_tally_matches(
    const stunrun_adsp_control_tally_t *tally)
{
    size_t i;
    if (tally == NULL)
        return 0;
    for (i = 0; i < STUNRUN_ADSP_CONTROL_OP_COUNT; i++) {
        if (tally->per_op[i] != kOps[i].expected_writes)
            return 0;
    }
    return tally->unknown_addresses == 0 && tally->data_mismatches == 0;
}
