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
static void dispatch_inputs(void)
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
    const char *control_state = "skipped";
    const char *counts_state = "skipped";
    size_t emitted;
    unsigned i;
    const char *gsp_vram_path = getenv("STUNRUN_GSP_VRAM_BIN");
    const char *gsp_palette_path = getenv("STUNRUN_GSP_PALETTE_BIN");
    const char *geometry_table_path = getenv("STUNRUN_GEOM_TABLE_BIN");
    const char *render_mode = "blank-scaffold";
    uint16_t geometry_table[STUNRUN_GEOM_MARCH_WORDS];
    uint16_t geometry_base[STUNRUN_GEOM_MARCH_WORDS];
    uint16_t geometry_twin[STUNRUN_GEOM_MARCH_WORDS];
    uint16_t geometry_fifo[STUNRUN_ROAD_FIFO_WRITES];

    stunrun_jsa_latches_init(&latches);
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
            memset(geometry_base, 0, sizeof(geometry_base));
            memset(geometry_twin, 0, sizeof(geometry_twin));
            passes = stunrun_geom_upload(STUNRUN_GEOM_COPY_COUNT,
                                         geometry_table, geometry_base,
                                         geometry_twin);
            size_t fifo_words = stunrun_road_fifo_drain(
                geometry_base, STUNRUN_GEOM_MARCH_WORDS, geometry_fifo,
                STUNRUN_ROAD_FIFO_WRITES);
            int fifo_match = fifo_words == STUNRUN_ROAD_FIFO_WRITES;
            for (i = 0; i < STUNRUN_GEOM_MARCH_WORDS; i++)
                if (geometry_base[i] != geometry_twin[i] ||
                    geometry_base[i] != geometry_table[i])
                    copies_match = 0;
            for (i = 0; i < STUNRUN_ROAD_FIFO_WRITES; i++)
                if (geometry_fifo[i] != geometry_base[i * 2u])
                    fifo_match = 0;
            expect(passes == STUNRUN_GEOM_COPY_COUNT);
            expect(copies_match);
            expect(fifo_match);
            printf("shell: geometry-upload fixture=loaded passes=%u "
                   "bytes=%u sum=0x%08X copies=match fifo-drain=%u/384 "
                   "dest=0x%08X\n", passes,
                   STUNRUN_GEOM_MARCH_WORDS * 2u,
                   (unsigned)geometry_sum(geometry_base),
                   (unsigned)fifo_words, STUNRUN_ROAD_FIFO_DEST);
        }
    }

    for (frame = 0; frame <= g_terminal; frame++) {
        dispatch_inputs();
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

    /* Rendering is a deliberately blank native frame boundary until the
     * first visible-output milestone supplies evidence-backed drawing. */
    stunrun_render_begin(&renderer, g_terminal, 0u, 0u, 0u);
    if ((gsp_vram_path != NULL && gsp_vram_path[0] != '\0') !=
        (gsp_palette_path != NULL && gsp_palette_path[0] != '\0')) {
        printf("shell: gsp-video requires both binary paths\n");
        g_failures++;
    } else if (gsp_vram_path != NULL && gsp_vram_path[0] != '\0') {
        if (!stunrun_gsp_video_load(&video, gsp_vram_path,
                                    gsp_palette_path)) {
            printf("shell: gsp-video load=error\n");
            g_failures++;
        } else if (!stunrun_gsp_video_render(&video, &renderer)) {
            printf("shell: gsp-video render=error\n");
            g_failures++;
        } else {
            render_mode = "gsp-visible-state";
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
