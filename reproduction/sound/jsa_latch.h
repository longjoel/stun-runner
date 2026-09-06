/* Atari JSA-II sound command/response latch transport, in C.
 *
 * Agent 2 (Implementer) encoding of Agent 1's M1 sound findings: the
 * 68010 <-> 6502 command/response path over the title path. Literal
 * mechanism only — byte payload semantics beyond the frame-447 pairing
 * remain unresolved and nothing here claims them.
 *
 * Provenance:
 * - Transport model: MAME-CONFIRMED via atariscom.cpp / atarijsa.cpp, see
 *   reference/experiments/stunrun/sound-handler-search.metadata.json:
 *   68010 main_command_w (0x600000) schedules a delayed command-latch
 *   write that asserts the JSA 6502 NMI; sound_command_r reads and clears
 *   the pending command at $2802. 6502 sound_response_w ($2A02) schedules
 *   a delayed response-latch write; the sound interrupt callback asserts
 *   main IRQ4, and main_response_r (0x600000) reads and clears it.
 * - Mirror mapping: MAME's 0x1F9 mirror mask places the ROM-observed alias
 *   $280A with $2802 sound_command_r; $280C is rdio_r, $280E is
 *   sound_irq_ack_r (same metadata file).
 * - Title-path vectors: reference/experiments/stunrun/sound-boundary-tap.
 *   metadata.json — identical classified counts in two independent
 *   600-frame runs, and the frame-447 pairing (68010 bus 0x1E1E/mask
 *   0xFF00 at PC 0x023EF6, then 6502 read at PC 0x5839/$280A = 0x1E).
 *
 * Deliberately NOT claimed:
 * - Only three JSA read addresses are classified ($2802/$280A command,
 *   $280C rdio, $280E irq-ack). Full mirror-class enumeration is not
 *   established; anything else reports OTHER.
 * - The command-byte extractor handles only the observed 0xFF00 mask
 *   event. Other bus masks are UNKNOWN, not zero.
 * - Empty-latch read values are unobserved (the ROM polls $280C status
 *   before consuming); the model reports "no byte" distinctly instead of
 *   inventing a value.
 * - Instruction-trace executions (e.g. 0x023EF6 x8) versus classified bus
 *   events (1 main command) are both recorded literally; reconciling them
 *   is an open Agent 1 question, so no relation is asserted.
 * - Producer-buffer ownership (0xFFDB64 region) and the 0x07 literal path
 *   (0x0300B8, zero executions in both traces) are STATIC-CANDIDATE
 *   context, not modeled behavior.
 *
 * No ROM contents are embedded or required.
 */
#ifndef STUNRUN_JSA_LATCH_H
#define STUNRUN_JSA_LATCH_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 68010-side window and PCs. */
#define STUNRUN_JSA_MAIN_WINDOW 0x600000u
#define STUNRUN_JSA_MAIN_COMMAND_PC 0x023EF6u
#define STUNRUN_JSA_MAIN_RESPONSE_READ_PC 0x023F66u
#define STUNRUN_JSA_MAIN_IRQ4_HANDLER_PC 0x023F24u
#define STUNRUN_JSA_MAIN_STATUS_TEST_ADDR 0xA80000u /* bit 15, static ctx */

/* 6502-side registers and observed PCs. */
#define STUNRUN_JSA_SOUND_COMMAND_REG 0x2802u
#define STUNRUN_JSA_SOUND_COMMAND_ALIAS 0x280Au
#define STUNRUN_JSA_RDIO_REG 0x280Cu
#define STUNRUN_JSA_IRQ_ACK_REG 0x280Eu
#define STUNRUN_JSA_SOUND_RESPONSE_REG 0x2A02u
#define STUNRUN_JSA_SOUND_POLL_PC 0x4154u
#define STUNRUN_JSA_SOUND_QUEUE_PC 0x4161u
#define STUNRUN_JSA_FRAME447_READ_PC 0x5839u

/* Frame-447 title-path command vector. */
#define STUNRUN_JSA_FRAME447_FRAME 447u
#define STUNRUN_JSA_FRAME447_BUS_DATA 0x1E1Eu
#define STUNRUN_JSA_FRAME447_BUS_MASK 0xFF00u
#define STUNRUN_JSA_FRAME447_BYTE 0x1Eu

/* Frame-1 startup response event (6502 PC 0x413E, $2A02 = 0xFF). */
#define STUNRUN_JSA_STARTUP_FRAME 1u
#define STUNRUN_JSA_STARTUP_PC 0x413Eu
#define STUNRUN_JSA_STARTUP_BYTE 0xFFu

/* No-byte sentinel for empty-latch reads (distinct from any data byte). */
#define STUNRUN_JSA_NO_BYTE (-1)

/* JSA read-address classes (only the three established addresses). */
typedef enum stunrun_jsa_read_class {
    STUNRUN_JSA_READ_COMMAND = 0,
    STUNRUN_JSA_READ_RDIO,
    STUNRUN_JSA_READ_IRQ_ACK,
    STUNRUN_JSA_READ_OTHER
} stunrun_jsa_read_class_t;

stunrun_jsa_read_class_t stunrun_jsa_classify_read(uint16_t address);

/* Nonzero when addr is the response register ($2A02). */
int stunrun_jsa_is_response_reg(uint16_t address);

/* Extract the command byte from an observed 68010 bus event. Handles the
 * observed 0xFF00 high-lane mask; returns STUNRUN_JSA_NO_BYTE for any
 * other mask (UNKNOWN, not zero). */
int stunrun_jsa_command_byte(unsigned bus_data, unsigned mask);

/* Two-latch transport state. Zero-initialized struct starts idle. */
typedef struct stunrun_jsa_latches {
    uint8_t command_byte;
    int command_pending;
    int nmi_asserted;
    uint8_t response_byte;
    int response_pending;
    int irq4_asserted;
} stunrun_jsa_latches_t;

void stunrun_jsa_latches_init(stunrun_jsa_latches_t *latches);

/* 68010 main_command_w: latch the byte, assert the 6502 NMI. */
void stunrun_jsa_main_command_write(stunrun_jsa_latches_t *latches,
                                    uint8_t byte);

/* 6502 sound_command_r: consume the pending command, clearing NMI.
 * Returns the byte, or STUNRUN_JSA_NO_BYTE when nothing is pending. */
int stunrun_jsa_sound_command_read(stunrun_jsa_latches_t *latches);

/* 6502 sound_response_w: latch the byte, assert main IRQ4. */
void stunrun_jsa_sound_response_write(stunrun_jsa_latches_t *latches,
                                      uint8_t byte);

/* 68010 main_response_r: consume the pending response, clearing IRQ4.
 * Returns the byte, or STUNRUN_JSA_NO_BYTE when nothing is pending. */
int stunrun_jsa_main_response_read(stunrun_jsa_latches_t *latches);

/* Title-path classified event counts, identical in both 600-frame runs. */
typedef struct stunrun_jsa_title_counts {
    unsigned main_command;
    unsigned main_response;
    unsigned sound_command;
    unsigned sound_response;
    unsigned sound_rdio;
    unsigned sound_irq_ack;
    unsigned sound_oki;
    unsigned sound_voice;
    unsigned sound_wrio;
    unsigned sound_mix;
    unsigned sound_other;
} stunrun_jsa_title_counts_t;

/* The observed title-path counts. */
stunrun_jsa_title_counts_t stunrun_jsa_observed_title_counts(void);

/* Nonzero iff counts equal the observed title-path table field by field. */
int stunrun_jsa_title_counts_match(const stunrun_jsa_title_counts_t *counts);

/* Observed 6502 command reads at the $280A alias (frame, pc, data). */
typedef struct stunrun_jsa_command_read {
    unsigned frame;
    uint16_t pc;
    uint8_t data;
} stunrun_jsa_command_read_t;

#define STUNRUN_JSA_COMMAND_READ_COUNT 4u

const stunrun_jsa_command_read_t *stunrun_jsa_command_reads(void);

#ifdef __cplusplus
}
#endif

#endif /* STUNRUN_JSA_LATCH_H */
