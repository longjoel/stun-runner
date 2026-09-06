/* arcade-experiment/v1 parser, dependency-free C99.
 *
 * Agent 2 (Implementer), M4 replay scaffolding. Parses the checked-in
 * experiment schema (schemas/experiment.schema.json) strictly enough for
 * deterministic replay: exact schema tag, id pattern, required keys, the
 * press/release/set action enum, set-requires-value, and
 * additionalProperties:false on events and expect. The optional
 * instrumentation subtree is accepted and skipped (never interpreted).
 *
 * Resource bounds (parser execution limits, not schema constraints):
 * file <= 256 KiB, events <= 1024, terminal frame <= 100000, strings as
 * sized below. Anything beyond reports STUNRUN_EXP_LIMIT.
 *
 * Only the "frame" expect kind is executable; "bounded_observation" parses
 * (terminal string exposed) but the shell refuses to run it with a clear
 * message instead of a silent pass.
 */
#ifndef STUNRUN_EXPERIMENT_H
#define STUNRUN_EXPERIMENT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STUNRUN_EXP_MAX_EVENTS 1024u
#define STUNRUN_EXP_MAX_FILE_BYTES (256u * 1024u)
#define STUNRUN_EXP_MAX_TERMINAL_FRAME 100000u
#define STUNRUN_EXP_ID_LEN 64u
#define STUNRUN_EXP_STR_LEN 96u

typedef enum stunrun_exp_action {
    STUNRUN_EXP_PRESS = 0,
    STUNRUN_EXP_RELEASE,
    STUNRUN_EXP_SET
} stunrun_exp_action_t;

typedef enum stunrun_exp_error {
    STUNRUN_EXP_OK = 0,
    STUNRUN_EXP_IO,
    STUNRUN_EXP_TOO_BIG,
    STUNRUN_EXP_SYNTAX,
    STUNRUN_EXP_SCHEMA,
    STUNRUN_EXP_LIMIT
} stunrun_exp_error_t;

typedef struct stunrun_exp_event {
    unsigned frame;
    char port[STUNRUN_EXP_STR_LEN];
    char field[STUNRUN_EXP_STR_LEN];
    stunrun_exp_action_t action;
    int has_value;
    int value;
    char label[STUNRUN_EXP_STR_LEN];
} stunrun_exp_event_t;

typedef enum stunrun_exp_expect_kind {
    STUNRUN_EXP_EXPECT_FRAME = 0,
    STUNRUN_EXP_EXPECT_BOUNDED_OBSERVATION
} stunrun_exp_expect_kind_t;

typedef struct stunrun_experiment {
    char id[STUNRUN_EXP_ID_LEN];
    char start[STUNRUN_EXP_STR_LEN];
    stunrun_exp_event_t events[STUNRUN_EXP_MAX_EVENTS];
    unsigned event_count;
    stunrun_exp_expect_kind_t expect_kind;
    unsigned terminal_frame;
    int has_timeout_frames;
    unsigned timeout_frames;
    char terminal[STUNRUN_EXP_STR_LEN];
} stunrun_experiment_t;

/* Parse the experiment file at path into out (must be non-NULL).
 * Returns STUNRUN_EXP_OK or the failure reason. */
stunrun_exp_error_t stunrun_experiment_parse(const char *path,
                                             stunrun_experiment_t *out);

const char *stunrun_experiment_error_string(stunrun_exp_error_t error);

#ifdef __cplusplus
}
#endif

#endif /* STUNRUN_EXPERIMENT_H */
