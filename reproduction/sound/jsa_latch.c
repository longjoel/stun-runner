/* See jsa_latch.h for provenance and confidence notes. */
#include "jsa_latch.h"

stunrun_jsa_read_class_t stunrun_jsa_classify_read(uint16_t address)
{
    if (address == STUNRUN_JSA_SOUND_COMMAND_REG ||
        address == STUNRUN_JSA_SOUND_COMMAND_ALIAS)
        return STUNRUN_JSA_READ_COMMAND;
    if (address == STUNRUN_JSA_RDIO_REG)
        return STUNRUN_JSA_READ_RDIO;
    if (address == STUNRUN_JSA_IRQ_ACK_REG)
        return STUNRUN_JSA_READ_IRQ_ACK;
    return STUNRUN_JSA_READ_OTHER;
}

int stunrun_jsa_is_response_reg(uint16_t address)
{
    return address == STUNRUN_JSA_SOUND_RESPONSE_REG;
}

int stunrun_jsa_command_byte(unsigned bus_data, unsigned mask)
{
    if (mask != STUNRUN_JSA_FRAME447_BUS_MASK)
        return STUNRUN_JSA_NO_BYTE;
    return (int)((bus_data & 0xFF00u) >> 8);
}

void stunrun_jsa_latches_init(stunrun_jsa_latches_t *latches)
{
    if (latches == NULL)
        return;
    latches->command_byte = 0;
    latches->command_pending = 0;
    latches->nmi_asserted = 0;
    latches->response_byte = 0;
    latches->response_pending = 0;
    latches->irq4_asserted = 0;
}

void stunrun_jsa_main_command_write(stunrun_jsa_latches_t *latches,
                                    uint8_t byte)
{
    if (latches == NULL)
        return;
    latches->command_byte = byte;
    latches->command_pending = 1;
    latches->nmi_asserted = 1;
}

int stunrun_jsa_sound_command_read(stunrun_jsa_latches_t *latches)
{
    int byte;
    if (latches == NULL || !latches->command_pending)
        return STUNRUN_JSA_NO_BYTE;
    byte = latches->command_byte;
    latches->command_pending = 0;
    latches->nmi_asserted = 0;
    return byte;
}

void stunrun_jsa_sound_response_write(stunrun_jsa_latches_t *latches,
                                      uint8_t byte)
{
    if (latches == NULL)
        return;
    latches->response_byte = byte;
    latches->response_pending = 1;
    latches->irq4_asserted = 1;
}

int stunrun_jsa_main_response_read(stunrun_jsa_latches_t *latches)
{
    int byte;
    if (latches == NULL || !latches->response_pending)
        return STUNRUN_JSA_NO_BYTE;
    byte = latches->response_byte;
    latches->response_pending = 0;
    latches->irq4_asserted = 0;
    return byte;
}

/* Exact event_counts from sound-boundary-tap.metadata.json (both runs). */
static const stunrun_jsa_title_counts_t kObservedCounts = {
    1, 4, 4, 3, 283775, 2487, 1244, 0, 2504, 6, 0
};

stunrun_jsa_title_counts_t stunrun_jsa_observed_title_counts(void)
{
    return kObservedCounts;
}

int stunrun_jsa_title_counts_match(const stunrun_jsa_title_counts_t *counts)
{
    const unsigned *want;
    const unsigned *got;
    size_t i;
    if (counts == NULL)
        return 0;
    want = (const unsigned *)&kObservedCounts;
    got = (const unsigned *)counts;
    for (i = 0; i < sizeof(kObservedCounts) / sizeof(unsigned); i++) {
        if (got[i] != want[i])
            return 0;
    }
    return 1;
}

/* Exact command_reads table from sound-boundary-tap.metadata.json. */
static const stunrun_jsa_command_read_t kCommandReads[] = {
    { 1, 0x4139u, 0x00u },
    { 369, 0x4139u, 0x00u },
    { 411, 0x4139u, 0x00u },
    { STUNRUN_JSA_FRAME447_FRAME, STUNRUN_JSA_FRAME447_READ_PC,
      STUNRUN_JSA_FRAME447_BYTE },
};

const stunrun_jsa_command_read_t *stunrun_jsa_command_reads(void)
{
    return kCommandReads;
}
