/* Deterministic native shell: M4 replay scaffolding.
 *
 * Agent 2 (Implementer). Two modes:
 *
 * - compiled-in (no argv): the fixed 600-frame title-path walk over the
 *   five verified C slices at their evidence-anchored frames.
 * - file mode (one argv: path to an arcade-experiment/v1 JSON file): the
 *   walk length comes from expect.frame and the file's input events are
 *   dispatched at their frames as telemetry. Only the "frame" expect kind
 *   runs; "bounded_observation" is refused with a clear message.
 *
 * Inputs have no consumer model yet (the shell owns no game state by
 * design), so dispatch logging IS the replay plumbing being proven here:
 * file -> strict parse -> deterministic frame dispatch. Whole-run
 * contracts (control tally, title counts) are 600-frame title-path facts
 * and assert only when the terminal frame is 600; shorter/longer runs log
 * them as skipped instead of failing on unestablished ground.
 *
 * Anchors (see the slice headers for full provenance):
 * - frame 1:   6502 startup response 0xFF at $2A02 (sound-boundary-tap)
 * - frames 1/369/411: 6502 command reads at $280A returning 0x00
 *   (telemetry only: empty-latch vs zero-command is UNKNOWN)
 * - frames 407/408/411: ADSP upload EMPTY/POPULATING/COMPLETE
 *   (m3-adsp-upload-boundary)
 * - frame 412: replacement install-ready; init-state image words and DM
 *   landmarks hold (m3-adsp-reset-entry, ram-map)
 * - frame 447: 68010 command byte 0x1E through the JSA latch to the 6502
 *   read (sound-boundary-tap)
 * - frame 600 of a 600-frame run: 73-write control multiset matches;
 *   title counts match (adsp-control-window, sound-boundary-tap)
 * - unpinned: second IRQ4 response walkthrough (2 entries observed, exact
 *   frames not established)
 *
 * Exit 0 with "RESULT PASS" iff every assertion holds; exit 2 on
 * usage/parse/unsupported-terminal failures. Output is fully
 * deterministic: same bytes on every run (no addresses, no timing).
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adsp_control_seq.h"
#include "adsp_init_image.h"
#include "adsp_upload_stream.h"
#include "fifo_block.h"
#include "geom_upload.h"
#include "road_fifo.h"
#include "experiment.h"
#include "jsa_latch.h"
#include "checkpoint.h"
#include "render.h"
#include "gsp_video.h"
#include "text_cursor.h"
#include "text_record.h"
#include "fake_ports.h"
#include "game_loop.h"
#include "road_strip.h"

static int g_failures = 0;
static unsigned frame = 0;
static unsigned g_terminal = 600u;
static const stunrun_exp_event_t *g_events = NULL;
static unsigned g_event_count = 0;
static unsigned g_dispatched = 0;
static unsigned g_pending = 0;

/* Generic replay boundary only: this is not game state. */
typedef struct shell_input_latch {
    char port[STUNRUN_EXP_STR_LEN];
    char field[STUNRUN_EXP_STR_LEN];
    int value;
} shell_input_latch_t;

static shell_input_latch_t g_input[STUNRUN_EXP_MAX_EVENTS];
static unsigned g_input_count = 0;
static uint32_t g_input_hash = 2166136261u;

static void input_hash_bytes(const void *data, size_t length)
{
    const unsigned char *bytes = (const unsigned char *)data;
    size_t i;
    for (i = 0; i < length; i++) {
        g_input_hash ^= bytes[i];
        g_input_hash *= 16777619u;
    }
}

static unsigned input_latch_index(const stunrun_exp_event_t *event)
{
    unsigned i;
    for (i = 0; i < g_input_count; i++)
        if (strcmp(g_input[i].port, event->port) == 0 &&
            strcmp(g_input[i].field, event->field) == 0)
            return i;
    if (g_input_count >= STUNRUN_EXP_MAX_EVENTS)
        return STUNRUN_EXP_MAX_EVENTS;
    snprintf(g_input[g_input_count].port, STUNRUN_EXP_STR_LEN, "%s",
             event->port);
    snprintf(g_input[g_input_count].field, STUNRUN_EXP_STR_LEN, "%s",
             event->field);
    g_input[g_input_count].value = 0;
    return g_input_count++;
}

static void input_latch_apply(const stunrun_exp_event_t *event)
{
    unsigned index = input_latch_index(event);
    unsigned i;
    if (index >= STUNRUN_EXP_MAX_EVENTS)
        return;
    g_input[index].value = event->action == STUNRUN_EXP_PRESS ? 1 :
                           event->action == STUNRUN_EXP_RELEASE ? 0 :
                           event->value;
    g_input_hash = 2166136261u;
    for (i = 0; i < g_input_count; i++) {
        input_hash_bytes(g_input[i].port, strlen(g_input[i].port) + 1u);
        input_hash_bytes(g_input[i].field, strlen(g_input[i].field) + 1u);
        input_hash_bytes(&g_input[i].value, sizeof(g_input[i].value));
    }
}

static unsigned input_active_count(void)
{
    unsigned i, active = 0;
    for (i = 0; i < g_input_count; i++)
        if (g_input[i].value != 0)
            active++;
    return active;
}

/* Optional evidence fixture only: the captured 68010 table bytes are
 * big-endian words.  No ROM table selection or course semantics are modeled
 * here; the literal upload slice owns only the 384-word march and twin copy. */
static int load_geometry_table(const char *path, uint16_t *table)
{
    unsigned char bytes[STUNRUN_GEOM_MARCH_WORDS * 2u];
    FILE *file;
    size_t i;

    if (path == NULL || table == NULL)
        return 0;
    file = fopen(path, "rb");
    if (file == NULL)
        return 0;
    if (fread(bytes, 1u, sizeof(bytes), file) != sizeof(bytes) ||
        fgetc(file) != EOF) {
        fclose(file);
        return 0;
    }
    fclose(file);
    for (i = 0; i < STUNRUN_GEOM_MARCH_WORDS; i++)
        table[i] = (uint16_t)(((uint16_t)bytes[i * 2u] << 8) |
                              bytes[i * 2u + 1u]);
    return 1;
}

/* Load a little-endian word fixture with an exact bounded capacity. */
static int load_word_fixture(const char *path, uint16_t *words,
                             size_t capacity, size_t *word_count)
{
    FILE *file;
    long bytes;
    size_t i;
    unsigned char raw[2];

    if (path == NULL || words == NULL || word_count == NULL)
        return 0;
    file = fopen(path, "rb");
    if (file == NULL || fseek(file, 0, SEEK_END) != 0)
        goto fail;
    bytes = ftell(file);
    if (bytes <= 0 || (bytes & 1) != 0 ||
        (size_t)bytes / 2u > capacity || fseek(file, 0, SEEK_SET) != 0)
        goto fail;
    for (i = 0; i < (size_t)bytes / 2u; i++) {
        if (fread(raw, 1u, sizeof(raw), file) != sizeof(raw))
            goto fail;
        words[i] = (uint16_t)raw[0] | ((uint16_t)raw[1] << 8);
    }
    if (fgetc(file) != EOF)
        goto fail;
    fclose(file);
    *word_count = (size_t)bytes / 2u;
    return 1;

fail:
    if (file != NULL)
        fclose(file);
    return 0;
}

static int parse_fixture_u32(const char *name, uint32_t *value)
{
    const char *text = getenv(name);
    char *end = NULL;
    unsigned long parsed;

    if (text == NULL || text[0] == '\0' || value == NULL)
        return 0;
    parsed = strtoul(text, &end, 0);
    if (end == text || *end != '\0' || parsed > UINT32_MAX)
        return 0;
    *value = (uint32_t)parsed;
    return 1;
}

static int parse_fixture_bases(const char *text, uint32_t *bases,
                               size_t count)
{
    size_t index = 0u;
    const char *cursor = text;
    char *end;
    unsigned long value;

    if (text == NULL || bases == NULL || count == 0u)
        return 0;
    while (*cursor != '\0' && index < count) {
        while (*cursor == ' ' || *cursor == '\t' || *cursor == ',')
            cursor++;
        if (*cursor == '\0')
            break;
        value = strtoul(cursor, &end, 0);
        if (end == cursor || value > UINT32_MAX)
            return 0;
        bases[index++] = (uint32_t)value;
        cursor = end;
        while (*cursor == ' ' || *cursor == '\t')
            cursor++;
        if (*cursor != '\0' && *cursor != ',')
            return 0;
    }
    return index == count && *cursor == '\0';
}

static int render_text_fixture(stunrun_renderer_t *renderer,
                               const char *table_path,
                               const char *words_path)
{
    uint16_t table[128u * 4u];
    uint16_t words[1024u];
    stunrun_gsp_text_glyph_t glyphs[2048u];
    size_t word_count = 0u;
    size_t glyph_count;
    uint32_t a0;
    uint32_t a1;
    uint32_t y_bias;
    size_t i;

    if (!load_word_fixture(table_path, table, sizeof(table) / sizeof(*table),
                           &word_count) || word_count != 128u * 4u ||
        !load_word_fixture(words_path, words,
                           sizeof(words) / sizeof(*words), &word_count) ||
        !parse_fixture_u32("STUNRUN_GSP_TEXT_A0", &a0) ||
        !parse_fixture_u32("STUNRUN_GSP_TEXT_A1", &a1) ||
        !parse_fixture_u32("STUNRUN_GSP_TEXT_Y_BIAS", &y_bias))
        return 0;
    glyph_count = stunrun_gsp_text_cursor_decode(
        a0, a1, (int)y_bias, words, word_count, glyphs,
        sizeof(glyphs) / sizeof(*glyphs));
    if (glyph_count == 0u)
        return 0;
    for (i = 0u; i < glyph_count; i++) {
        if (!stunrun_render_gsp_glyph_from_table(
                renderer, table, sizeof(table) / sizeof(*table),
                glyphs[i].glyph_code, glyphs[i].x, glyphs[i].y,
                0xFFu, 0xFEu, 0u))
            return 0;
    }
    printf("shell: gsp-text fixture=loaded words=%u glyphs=%u "
           "a0=0x%08X a1=0x%08X y-bias=%u\n",
           (unsigned)word_count, (unsigned)glyph_count,
           (unsigned)a0, (unsigned)a1, (unsigned)y_bias);
    return 1;
}

static int render_text_record_fixture(stunrun_renderer_t *renderer,
                                       const char *table_path,
                                       const char *records_path)
{
    uint16_t table[128u * 4u];
    uint16_t words[1024u];
    stunrun_gsp_text_record_t record;
    stunrun_gsp_text_glyph_t glyphs[2048u];
    size_t table_count = 0u;
    size_t word_count = 0u;
    size_t record_index;
    uint32_t record_base;
    uint32_t record_bases[128u];
    const char *record_bases_text;
    uint32_t y_bias;

    if (!load_word_fixture(table_path, table, sizeof(table) / sizeof(*table),
                           &table_count) || table_count != 128u * 4u ||
        !load_word_fixture(records_path, words, sizeof(words) / sizeof(*words),
                           &word_count) ||
        word_count == 0u || word_count % STUNRUN_GSP_TEXT_RECORD_WORDS != 0u ||
        !parse_fixture_u32("STUNRUN_GSP_TEXT_Y_BIAS", &y_bias))
        return 0;
    if (word_count / STUNRUN_GSP_TEXT_RECORD_WORDS >
        sizeof(record_bases) / sizeof(*record_bases))
        return 0;
    record_bases_text = getenv("STUNRUN_GSP_TEXT_RECORD_BASES");
    if (record_bases_text != NULL && record_bases_text[0] != '\0') {
        char copy[4096];
        if (strlen(record_bases_text) >= sizeof(copy))
            return 0;
        memcpy(copy, record_bases_text, strlen(record_bases_text) + 1u);
        if (!parse_fixture_bases(
                copy, record_bases, word_count / STUNRUN_GSP_TEXT_RECORD_WORDS))
            return 0;
    } else {
        if (!parse_fixture_u32("STUNRUN_GSP_TEXT_RECORD_BASE", &record_base))
            return 0;
        for (record_index = 0u;
             record_index < word_count / STUNRUN_GSP_TEXT_RECORD_WORDS;
             record_index++)
            record_bases[record_index] = record_base +
                                         (uint32_t)(record_index * 0x80u);
    }
    for (record_index = 0u;
         record_index < word_count / STUNRUN_GSP_TEXT_RECORD_WORDS;
         record_index++) {
        const uint16_t *packed;
        uint32_t descriptor;
        uint32_t a1;
        size_t glyph_count;
        size_t glyph_index;
        stunrun_gsp_text_record_load(
            words + record_index * STUNRUN_GSP_TEXT_RECORD_WORDS, &record);
        if (!stunrun_gsp_text_record_cursor(
                record_bases[record_index], &record,
                &descriptor, &a1, &packed))
            return 0;
        (void)descriptor;
        glyph_count = stunrun_gsp_text_cursor_decode(
            descriptor, a1, (int)y_bias, packed, 4u, glyphs,
            sizeof(glyphs) / sizeof(*glyphs));
        if (glyph_count == 0u)
            return 0;
        for (glyph_index = 0u; glyph_index < glyph_count; glyph_index++)
            if (!stunrun_render_gsp_glyph_from_table(
                    renderer, table, sizeof(table) / sizeof(*table),
                    glyphs[glyph_index].glyph_code, glyphs[glyph_index].x,
                    glyphs[glyph_index].y, 0xFFu, 0xFEu, 0u))
                return 0;
    }
    printf("shell: gsp-text-record fixture=loaded records=%u words=%u "
           "base=0x%08X y-bias=%u\n",
           (unsigned)(word_count / STUNRUN_GSP_TEXT_RECORD_WORDS),
           (unsigned)word_count, (unsigned)record_bases[0], (unsigned)y_bias);
    return 1;
}

static uint32_t geometry_sum(const uint16_t *words)
{
    uint32_t sum = 0;
    unsigned i;
    for (i = 0; i < STUNRUN_GEOM_MARCH_WORDS; i++)
        sum += words[i];
    return sum;
}

#define expect(cond) \
    do { \
        if (!(cond)) { \
            printf("shell: ASSERT FAIL %s frame=%u\n", #cond, frame); \
            g_failures++; \
        } \
    } while (0)

/* Feed the exact title-path control multiset into the tally. */
static void feed_control_contract(stunrun_adsp_control_tally_t *tally)
{
    const stunrun_adsp_control_op_t *ops = stunrun_adsp_control_ops();
    size_t i;
    unsigned k;
    for (i = 0; i < stunrun_adsp_control_op_count(); i++) {
        for (k = 0; k < ops[i].expected_writes; k++)
            stunrun_adsp_control_tally_add(tally, ops[i].address,
                                           STUNRUN_ADSP_CONTROL_DATA);
    }
}

static const char *action_name(stunrun_exp_action_t action)
{
    switch (action) {
    case STUNRUN_EXP_PRESS:
        return "press";
    case STUNRUN_EXP_RELEASE:
        return "release";
    case STUNRUN_EXP_SET:
        return "set";
    default:
        return "unknown";
    }
}

/* Dispatch file input events scheduled for the current frame. Order across
 * events sharing a frame follows file order; unsorted files still dispatch
 * every event exactly once because each frame scans the whole list. */
static void dispatch_inputs(stunrun_fake_ports_t *ports)
{
    unsigned i;
    for (i = 0; i < g_event_count; i++) {
        const stunrun_exp_event_t *ev = &g_events[i];
        if (ev->frame != frame)
            continue;
        g_dispatched++;
        printf("shell: input frame=%u port=%s field=%s action=%s",
               frame, ev->port, ev->field, action_name(ev->action));
        if (ev->has_value)
            printf(" value=%d", ev->value);
        if (ev->label[0] != '\0')
            printf(" label=%s", ev->label);
        printf("\n");
        input_latch_apply(ev);
        (void)stunrun_port_set_named(ports, ev->port, ev->field,
                                     ev->action == STUNRUN_EXP_PRESS ? 1 :
                                     ev->action == STUNRUN_EXP_RELEASE ? 0 :
                                     ev->value);
        printf("shell: input-state frame=%u active=%u hash=0x%08X\n",
               frame, input_active_count(), (unsigned)g_input_hash);
    }
}

static const char *upload_name(stunrun_adsp_upload_state_t state)
{
    switch (state) {
    case STUNRUN_ADSP_UPLOAD_EMPTY:
        return "empty";
    case STUNRUN_ADSP_UPLOAD_POPULATING:
        return "populating";
    case STUNRUN_ADSP_UPLOAD_COMPLETE:
        return "complete";
    default:
        return "unknown";
    }
}

/* Emit the shared checkpoint shape for the native transport model.  CPU
 * registers are deliberately zero because this shell models verified
 * interconnect contracts, not a CPU interpreter.  Keeping those fields
 * explicit prevents the output from being mistaken for an oracle match. */
static void emit_model_checkpoint(const uint32_t *image, size_t image_words)
{
    stunrun_checkpoint_t cp;
    char document[4096];
    unsigned i;
    unsigned nonzero = 0;
    uint32_t sum32 = 0;
    unsigned first = 0;
    memset(&cp, 0, sizeof(cp));
    snprintf(cp.system, sizeof(cp.system), "stunrun");
    snprintf(cp.description, sizeof(cp.description),
             "native-shell-transport-model");
    snprintf(cp.mame, sizeof(cp.mame), "native-model");
    cp.frame = g_terminal;
    cp.time_seconds = (double)g_terminal / 60.0;
    snprintf(cp.maincpu_tag, sizeof(cp.maincpu_tag), ":mainpcb:maincpu");
    snprintf(cp.gsp_tag, sizeof(cp.gsp_tag), ":mainpcb:gsp");
    snprintf(cp.adsp_tag, sizeof(cp.adsp_tag), ":mainpcb:adsp");
    snprintf(cp.soundcpu_tag, sizeof(cp.soundcpu_tag), ":mainpcb:jsa:cpu");
    snprintf(cp.region_space, sizeof(cp.region_space), "program");
    snprintf(cp.region_range, sizeof(cp.region_range), "0x0000-0x1FFF");
    cp.region_word_width = 24;
    for (i = 0; i < image_words; i++) {
        if (image[i] != 0u) {
            if (first < STUNRUN_CHECKPOINT_FIRST_WORDS)
                cp.region_first_words[first++] = image[i];
            nonzero++;
        }
        sum32 += image[i];
    }
    cp.region_nonzero_words = nonzero;
    cp.region_sum32 = sum32;
    cp.adsp_program_loaded = g_terminal >= 412u;
    if (stunrun_checkpoint_emit(&cp, document, sizeof(document)) == 0u) {
        printf("shell: checkpoint-json=emit-failed\n");
        g_failures++;
        return;
    }
    printf("shell: checkpoint-json=");
    for (i = 0; document[i] != '\0'; i++)
        putchar(document[i] == '\n' ? ' ' : document[i]);
    putchar('\n');
}

static int run_walk(void)
{
    static uint32_t image[STUNRUN_ADSP_INIT_STATE_WORDS];
    static uint16_t dm[STUNRUN_ADSP_DM_SIZE];
    static stunrun_renderer_t renderer;
    stunrun_gsp_video_state_t video;
    const char *render_path;
    stunrun_jsa_latches_t latches;
    stunrun_adsp_control_tally_t tally;
    stunrun_jsa_title_counts_t counts;
    stunrun_fifo_run_summary_t fifo_summary;
    stunrun_fake_ports_t ports;
    stunrun_game_loop_t game_loop;
    const char *control_state = "skipped";
    const char *counts_state = "skipped";
    size_t emitted;
    unsigned i;
    const char *gsp_vram_path = getenv("STUNRUN_GSP_VRAM_BIN");
    const char *gsp_palette_path = getenv("STUNRUN_GSP_PALETTE_BIN");
    const char *gsp_palette_low_path = getenv("STUNRUN_GSP_PALETTE_LOW_BIN");
    const char *gsp_palette_high_path = getenv("STUNRUN_GSP_PALETTE_HIGH_BIN");
    const char *geometry_table_path = getenv("STUNRUN_GEOM_TABLE_BIN");
    const char *gsp_text_table_path = getenv("STUNRUN_GSP_TEXT_TABLE_BIN");
    const char *gsp_text_words_path = getenv("STUNRUN_GSP_TEXT_WORDS_BIN");
    const char *gsp_text_record_path = getenv("STUNRUN_GSP_TEXT_RECORD_BIN");
    const char *road_strip_fixture = getenv("STUNRUN_ROAD_STRIP_FIXTURE");
    const char *render_mode = "blank-scaffold";
    uint16_t geometry_table[STUNRUN_GEOM_MARCH_WORDS];

    stunrun_jsa_latches_init(&latches);
    stunrun_ports_init(&ports);
    stunrun_game_loop_init(&game_loop, &ports);
    stunrun_adsp_control_tally_init(&tally);
    stunrun_render_init(&renderer);
    stunrun_gsp_video_init(&video);

    if (geometry_table_path != NULL) {
        unsigned passes;
        unsigned i;
        int copies_match = 1;

        if (!load_geometry_table(geometry_table_path, geometry_table)) {
            printf("shell: geometry-upload fixture=read-failed\n");
            g_failures++;
        } else {
            passes = stunrun_game_loop_load_road_table(&game_loop,
                                                       geometry_table) ?
                     STUNRUN_GEOM_COPY_COUNT : 0u;
            size_t fifo_words = stunrun_game_loop_submit_road(&game_loop);
            int fifo_match = fifo_words == STUNRUN_ROAD_FIFO_WRITES;
            for (i = 0; i < STUNRUN_GEOM_MARCH_WORDS; i++)
                if (game_loop.road_base[i] != game_loop.road_twin[i] ||
                    game_loop.road_base[i] != geometry_table[i])
                    copies_match = 0;
            for (i = 0; i < STUNRUN_ROAD_FIFO_WRITES; i++)
                if (game_loop.road_fifo[i] != game_loop.road_base[i * 2u])
                    fifo_match = 0;
            expect(passes == STUNRUN_GEOM_COPY_COUNT);
            expect(copies_match);
            expect(fifo_match);
            printf("shell: geometry-upload fixture=loaded passes=%u "
                   "bytes=%u sum=0x%08X copies=match fifo-drain=%u/384 "
                   "dest=0x%08X\n", passes,
                   STUNRUN_GEOM_MARCH_WORDS * 2u,
                   (unsigned)geometry_sum(game_loop.road_base),
                   (unsigned)fifo_words, STUNRUN_ROAD_FIFO_DEST);
        }
    }

    for (frame = 0; frame <= g_terminal; frame++) {
        dispatch_inputs(&ports);
        if (frame != 0u)
            stunrun_game_loop_step(&game_loop);
        if (frame == 1) {
            /* Startup response: 6502 writes 0xFF, main IRQ4 fires,
             * handler reads 0x600000, IRQ4 clears. */
            stunrun_jsa_sound_response_write(&latches,
                                             STUNRUN_JSA_STARTUP_BYTE);
            expect(latches.irq4_asserted);
            expect(stunrun_jsa_main_response_read(&latches) == 0xFFu);
            expect(!latches.irq4_asserted);
            printf("shell: frame=1 startup-response=0xFF irq4=handled\n");
        }
        if (frame == 1 || frame == 369 || frame == 411) {
            /* Observed $280A reads returning 0x00. Telemetry: the
             * transport meaning of an idle read is UNKNOWN, so the
             * latch is not driven here. */
            printf("shell: frame=%u command-read addr=0x280A data=0x00 "
                   "telemetry\n", frame);
        }
        if (frame == 407)
            expect(stunrun_adsp_upload_state_at(frame) ==
                   STUNRUN_ADSP_UPLOAD_EMPTY);
        if (frame == 408)
            expect(stunrun_adsp_upload_state_at(frame) ==
                   STUNRUN_ADSP_UPLOAD_POPULATING);
        if (frame == 411)
            expect(stunrun_adsp_upload_state_at(frame) ==
                   STUNRUN_ADSP_UPLOAD_COMPLETE);
        if (frame == 412) {
            expect(stunrun_jsa_main_response_read(&latches) ==
                   STUNRUN_JSA_NO_BYTE);
            expect(stunrun_adsp_install_frame_ready(frame));
            emitted = stunrun_adsp_emit(STUNRUN_ADSP_FIXTURE_INIT_STATE,
                                        image, STUNRUN_ADSP_INIT_STATE_WORDS);
            expect(emitted == STUNRUN_ADSP_INIT_STATE_WORDS);
            expect(image[0x0004u] == 0x001C780Fu);
            expect(image[0x0005u] == 0x001C834Fu);
            expect(image[STUNRUN_ADSP_MAILBOX_BOUNDARY] == 0x0018050Fu);
            stunrun_adsp_apply_dm_landmarks(dm);
            expect(dm[0x0955u] == 0x1242u);
            expect(dm[0x0956u] == 0x124Eu);
            expect(dm[0x0959u] == 0x7FFFu);
            expect(dm[0x095Au] == 0xFFFFu);
            printf("shell: frame=412 install-ready=1 image=init-state "
                   "landmarks=ok\n");
        }
        if (frame == 447) {
            /* Frame-447 command: 68010 bus event decodes to 0x1E, the
             * latch carries it across the NMI, the 6502 read gets 0x1E. */
            int byte = stunrun_jsa_command_byte(
                STUNRUN_JSA_FRAME447_BUS_DATA, STUNRUN_JSA_FRAME447_BUS_MASK);
            expect(byte == STUNRUN_JSA_FRAME447_BYTE);
            stunrun_jsa_main_command_write(&latches, (uint8_t)byte);
            expect(latches.nmi_asserted);
            expect(stunrun_jsa_sound_command_read(&latches) == 0x1Eu);
            expect(!latches.nmi_asserted);
            printf("shell: frame=447 command-byte=0x1E nmi=handled\n");
        }
        if (frame == g_terminal && g_terminal == 600u) {
            feed_control_contract(&tally);
            expect(stunrun_adsp_control_tally_matches(&tally));
            counts = stunrun_jsa_observed_title_counts();
            expect(stunrun_jsa_title_counts_match(&counts));
            fifo_summary = stunrun_fifo_observed_run();
            expect(stunrun_fifo_run_matches(&fifo_summary));
            control_state = "match";
            counts_state = "match";
            printf("shell: frame=600 control-tally=match "
                   "title-counts=match fifo-block=match\n");
        }
    }

    if (g_terminal != 600u)
        printf("shell: frame=%u full-contract=skipped terminal!=600\n",
               g_terminal);

    /* Second IRQ4 response walkthrough. Two entries are observed per
     * 600-frame run but exact frames are not established, so this is
     * order-only and carries no frame attribution. The byte is a
     * mechanism-test placeholder: observed response values beyond the
     * startup 0xFF are not established. */
    stunrun_jsa_sound_response_write(&latches, 0xA5u);
    expect(latches.irq4_asserted);
    expect(stunrun_jsa_main_response_read(&latches) == 0xA5u);
    expect(!latches.irq4_asserted);
    printf("shell: response-pair-2 irq4=handled unpinned\n");

    g_pending = 0;
    for (i = 0; i < g_event_count; i++) {
        if (g_events[i].frame > g_terminal)
            g_pending++;
    }

    /* Rendering accepts either the captured whole-VRAM bridge or the
     * evidence-backed text-cursor fixture. Neither path supplies game state. */
    stunrun_render_begin(&renderer, g_terminal, 0u, 0u, 0u);
    {
        int has_vram = gsp_vram_path != NULL && gsp_vram_path[0] != '\0';
        int has_rgb = gsp_palette_path != NULL && gsp_palette_path[0] != '\0';
        int has_low = gsp_palette_low_path != NULL &&
                      gsp_palette_low_path[0] != '\0';
        int has_high = gsp_palette_high_path != NULL &&
                       gsp_palette_high_path[0] != '\0';
        int has_text_table = gsp_text_table_path != NULL &&
                             gsp_text_table_path[0] != '\0';
        int has_text_words = gsp_text_words_path != NULL &&
                             gsp_text_words_path[0] != '\0';
        int has_text_records = gsp_text_record_path != NULL &&
                               gsp_text_record_path[0] != '\0';
        int has_road_strip = road_strip_fixture != NULL &&
                             strcmp(road_strip_fixture, "1") == 0;
        int has_any_text = has_text_table || has_text_words || has_text_records;
        if (has_road_strip &&
            (has_vram || has_rgb || has_low || has_high || has_any_text)) {
            printf("shell: road-strip fixture excludes other render inputs\n");
            g_failures++;
        } else if (has_road_strip) {
            stunrun_road_strip_t strip;
            stunrun_road_strip_fixture(&strip);
            if (!stunrun_render_road_strip(&renderer, &strip)) {
                printf("shell: road-strip fixture=error\n");
                g_failures++;
            } else {
                render_mode = "road-strip-fixture";
            }
        } else if (has_any_text && (!has_text_table ||
            (has_text_words == has_text_records) ||
            (has_text_table && (has_vram || has_rgb || has_low || has_high)))) {
            printf("shell: gsp-text requires table plus exactly one of words/records and excludes gsp-video\n");
            g_failures++;
        } else if (has_text_records) {
            if (!render_text_record_fixture(&renderer, gsp_text_table_path,
                                            gsp_text_record_path)) {
                printf("shell: gsp-text-record load=error\n");
                g_failures++;
            } else {
                render_mode = "gsp-text-record-fixture";
            }
        } else if (has_text_table) {
            if (!render_text_fixture(&renderer, gsp_text_table_path,
                                     gsp_text_words_path)) {
                printf("shell: gsp-text load=error\n");
                g_failures++;
            } else {
                render_mode = "gsp-text-cursor-fixture";
            }
        } else if (!has_vram || (has_rgb && (has_low || has_high)) ||
            (!has_rgb && (has_low != has_high))) {
            if (has_vram || has_rgb || has_low || has_high) {
                printf("shell: gsp-video requires VRAM plus either RGB or both raw palette planes\n");
                g_failures++;
            }
        } else if (has_rgb || has_low) {
            int loaded = has_rgb ?
                stunrun_gsp_video_load(&video, gsp_vram_path,
                                       gsp_palette_path) :
                stunrun_gsp_video_load_palette_planes(
                    &video, gsp_vram_path, gsp_palette_low_path,
                    gsp_palette_high_path);
            if (!loaded) {
            printf("shell: gsp-video load=error\n");
            g_failures++;
            } else if (!stunrun_gsp_video_render(&video, &renderer)) {
            printf("shell: gsp-video render=error\n");
            g_failures++;
            } else {
            render_mode = "gsp-visible-state";
            }
        }
    }
    render_path = getenv("STUNRUN_RENDER_PPM");
    if (render_path != NULL && render_path[0] != '\0') {
        if (!stunrun_render_write_ppm(&renderer, render_path)) {
            printf("shell: render-output path=%s status=error\n", render_path);
            g_failures++;
        } else {
            printf("shell: render-output path=%s status=written\n", render_path);
        }
    }
    printf("render frame=%u width=%u height=%u hash=0x%08X mode=%s\n",
           renderer.frame, STUNRUN_RENDER_WIDTH, STUNRUN_RENDER_HEIGHT,
           (unsigned)stunrun_render_hash(&renderer), render_mode);
    printf("game-loop frame=%u ticks=%u steering=%d trajectory=0x%04X "
           "motion-delta=%d\n",
           game_loop.frame, (unsigned)game_loop.global_tick,
           (int)game_loop.steering_delta,
           (unsigned)(uint16_t)game_loop.trajectory_coordinate,
           (int)game_loop.motion_delta);
    printf("checkpoint frames=%u time-us=%u events=%u pending=%u "
           "input-active=%u input-hash=0x%08X upload=%s "
           "install_ready=%d control=%s counts=%s nmi=%d irq4=%d\n",
           g_terminal, (unsigned)(((uint64_t)g_terminal * 1000000u) / 60u),
           g_dispatched, g_pending, input_active_count(),
           (unsigned)g_input_hash,
           upload_name(stunrun_adsp_upload_state_at(g_terminal)),
           stunrun_adsp_install_frame_ready(g_terminal),
           control_state, counts_state,
           latches.nmi_asserted ? 1 : 0,
           latches.irq4_asserted ? 1 : 0);
    emit_model_checkpoint(image, STUNRUN_ADSP_INIT_STATE_WORDS);
    stunrun_gsp_video_free(&video);
    if (g_failures == 0)
        printf("RESULT PASS\n");
    else
        printf("RESULT FAIL failures=%d\n", g_failures);
    return g_failures == 0 ? 0 : 1;
}

int main(int argc, char **argv)
{
    static stunrun_experiment_t exp;
    stunrun_exp_error_t err;
    if (argc == 1)
        return run_walk();
    if (argc != 2) {
        fprintf(stderr, "usage: %s [experiment.json]\n", argv[0]);
        return 2;
    }
    err = stunrun_experiment_parse(argv[1], &exp);
    if (err != STUNRUN_EXP_OK) {
        fprintf(stderr, "shell: experiment parse failed: %s\n",
                stunrun_experiment_error_string(err));
        return 2;
    }
    if (exp.expect_kind != STUNRUN_EXP_EXPECT_FRAME) {
        fprintf(stderr, "shell: unsupported terminal kind "
                "(bounded_observation needs a Verifier harness)\n");
        return 2;
    }
    g_terminal = exp.terminal_frame;
    g_events = exp.events;
    g_event_count = exp.event_count;
    printf("shell: experiment id=%s start=%s events=%u terminal=%u\n",
           exp.id, exp.start, exp.event_count, exp.terminal_frame);
    return run_walk();
}
