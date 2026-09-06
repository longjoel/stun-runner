/* ROM-free self-check for the arcade-experiment/v1 C parser.
 *
 * Writes small JSON fixtures to /tmp, parses them, and checks the
 * verdict plus decoded fields. Exit 0 on success.
 */
#include <stdio.h>
#include <string.h>

#include "experiment.h"

static int g_failures = 0;

#define check(cond) \
    do { \
        if (cond) { \
            printf("PASS %s\n", #cond); \
        } else { \
            printf("FAIL %s\n", #cond); \
            g_failures++; \
        } \
    } while (0)

static int write_fixture(const char *name, const char *body)
{
    static char path[128];
    FILE *fp;
    snprintf(path, sizeof(path), "/tmp/stunrun-exp-test-%s.json", name);
    fp = fopen(path, "wb");
    if (fp == NULL)
        return 0;
    fputs(body, fp);
    fclose(fp);
    return 1;
}

static stunrun_exp_error_t parse_fixture(const char *name,
                                         stunrun_experiment_t *exp)
{
    static char path[128];
    snprintf(path, sizeof(path), "/tmp/stunrun-exp-test-%s.json", name);
    return stunrun_experiment_parse(path, exp);
}

int main(void)
{
    static stunrun_experiment_t exp;
    stunrun_exp_error_t err;

    check(write_fixture("valid",
        "{\"schema\":\"arcade-experiment/v1\",\"id\":\"boot_to_title\","
        "\"start\":\"power_on\",\"events\":[],"
        "\"expect\":{\"kind\":\"frame\",\"frame\":600,\"timeout_frames\":720}}"));
    err = parse_fixture("valid", &exp);
    check(err == STUNRUN_EXP_OK);
    check(strcmp(exp.id, "boot_to_title") == 0);
    check(strcmp(exp.start, "power_on") == 0);
    check(exp.event_count == 0u);
    check(exp.expect_kind == STUNRUN_EXP_EXPECT_FRAME);
    check(exp.terminal_frame == 600u);
    check(exp.has_timeout_frames && exp.timeout_frames == 720u);

    check(write_fixture("inputs",
        "{\"schema\":\"arcade-experiment/v1\",\"id\":\"drive-1\","
        "\"start\":\"power_on\",\"events\":["
        "{\"frame\":900,\"port\":\":mainpcb:IN0\",\"field\":\"Coin 1\","
        " \"action\":\"press\",\"label\":\"coin\"},"
        "{\"frame\":902,\"port\":\":mainpcb:IN0\",\"field\":\"Coin 1\","
        " \"action\":\"release\"},"
        "{\"frame\":900,\"port\":\":mainpcb:8BADC.0\",\"field\":\"AD Stick X\","
        " \"action\":\"set\",\"value\":220}],"
        "\"expect\":{\"kind\":\"frame\",\"frame\":1800},"
        "\"instrumentation\":{\"tool\":\"x\",\"frames\":6}}"));
    err = parse_fixture("inputs", &exp);
    check(err == STUNRUN_EXP_OK);
    check(exp.event_count == 3u);
    check(exp.events[0].frame == 900u);
    check(exp.events[0].action == STUNRUN_EXP_PRESS);
    check(strcmp(exp.events[0].label, "coin") == 0);
    check(!exp.events[0].has_value);
    check(exp.events[2].action == STUNRUN_EXP_SET);
    check(exp.events[2].has_value && exp.events[2].value == 220);
    check(strcmp(exp.events[2].port, ":mainpcb:8BADC.0") == 0);
    check(!exp.has_timeout_frames);

    check(write_fixture("obs",
        "{\"schema\":\"arcade-experiment/v1\",\"id\":\"watch\","
        "\"start\":\"power_on\",\"events\":[],"
        "\"expect\":{\"kind\":\"bounded_observation\",\"terminal\":\"title\"}}"));
    err = parse_fixture("obs", &exp);
    check(err == STUNRUN_EXP_OK);
    check(exp.expect_kind == STUNRUN_EXP_EXPECT_BOUNDED_OBSERVATION);
    check(strcmp(exp.terminal, "title") == 0);

    check(write_fixture("badtag",
        "{\"schema\":\"arcade-experiment/v0\",\"id\":\"x\",\"start\":\"s\","
        "\"events\":[],\"expect\":{\"kind\":\"frame\",\"frame\":1}}"));
    check(parse_fixture("badtag", &exp) == STUNRUN_EXP_SCHEMA);

    check(write_fixture("badid",
        "{\"schema\":\"arcade-experiment/v1\",\"id\":\"Bad Id\","
        "\"start\":\"s\",\"events\":[],"
        "\"expect\":{\"kind\":\"frame\",\"frame\":1}}"));
    check(parse_fixture("badid", &exp) == STUNRUN_EXP_SCHEMA);

    check(write_fixture("badkey",
        "{\"schema\":\"arcade-experiment/v1\",\"id\":\"x\",\"start\":\"s\","
        "\"events\":[{\"frame\":1,\"port\":\"p\",\"field\":\"f\","
        "\"action\":\"press\",\"extra\":1}],"
        "\"expect\":{\"kind\":\"frame\",\"frame\":1}}"));
    check(parse_fixture("badkey", &exp) == STUNRUN_EXP_SCHEMA);

    check(write_fixture("novalue",
        "{\"schema\":\"arcade-experiment/v1\",\"id\":\"x\",\"start\":\"s\","
        "\"events\":[{\"frame\":1,\"port\":\"p\",\"field\":\"f\","
        "\"action\":\"set\"}],"
        "\"expect\":{\"kind\":\"frame\",\"frame\":1}}"));
    check(parse_fixture("novalue", &exp) == STUNRUN_EXP_SCHEMA);

    check(write_fixture("badaction",
        "{\"schema\":\"arcade-experiment/v1\",\"id\":\"x\",\"start\":\"s\","
        "\"events\":[{\"frame\":1,\"port\":\"p\",\"field\":\"f\","
        "\"action\":\"hold\"}],"
        "\"expect\":{\"kind\":\"frame\",\"frame\":1}}"));
    check(parse_fixture("badaction", &exp) == STUNRUN_EXP_SCHEMA);

    check(write_fixture("negframe",
        "{\"schema\":\"arcade-experiment/v1\",\"id\":\"x\",\"start\":\"s\","
        "\"events\":[{\"frame\":-1,\"port\":\"p\",\"field\":\"f\","
        "\"action\":\"press\"}],"
        "\"expect\":{\"kind\":\"frame\",\"frame\":1}}"));
    check(parse_fixture("negframe", &exp) == STUNRUN_EXP_SCHEMA);

    check(write_fixture("noexpect",
        "{\"schema\":\"arcade-experiment/v1\",\"id\":\"x\",\"start\":\"s\","
        "\"events\":[]}"));
    check(parse_fixture("noexpect", &exp) == STUNRUN_EXP_SCHEMA);

    check(write_fixture("topkey",
        "{\"schema\":\"arcade-experiment/v1\",\"id\":\"x\",\"start\":\"s\","
        "\"events\":[],\"expect\":{\"kind\":\"frame\",\"frame\":1},"
        "\"bogus\":2}"));
    check(parse_fixture("topkey", &exp) == STUNRUN_EXP_SCHEMA);

    check(write_fixture("trailing",
        "{\"schema\":\"arcade-experiment/v1\",\"id\":\"x\",\"start\":\"s\","
        "\"events\":[],\"expect\":{\"kind\":\"frame\",\"frame\":1}} }"));
    check(parse_fixture("trailing", &exp) == STUNRUN_EXP_SYNTAX);

    check(write_fixture("trunc",
        "{\"schema\":\"arcade-experiment/v1\",\"id\":\"x\",\"start\":"));
    check(parse_fixture("trunc", &exp) == STUNRUN_EXP_SYNTAX);

    /* Escape handling: \" \\\\ newline and \\u0041 decode; a bad escape,
     * a non-ASCII \\u, and an unterminated string are SYNTAX. */
    check(write_fixture("escapes",
        "{\"schema\":\"arcade-experiment/v1\",\"id\":\"x\",\"start\":\"s\","
        "\"events\":[{\"frame\":1,\"port\":\"p\",\"field\":\"f\","
        "\"action\":\"press\",\"label\":\"a\\\"b\\\\c\\nd\\u0041\"}],"
        "\"expect\":{\"kind\":\"frame\",\"frame\":1}}"));
    check(parse_fixture("escapes", &exp) == STUNRUN_EXP_OK);
    check(strcmp(exp.events[0].label, "a\"b\\c\ndA") == 0);

    check(write_fixture("badesc",
        "{\"schema\":\"arcade-experiment/v1\",\"id\":\"x\",\"start\":\"s\","
        "\"events\":[{\"frame\":1,\"port\":\"p\",\"field\":\"f\","
        "\"action\":\"press\",\"label\":\"a\\xb\"}],"
        "\"expect\":{\"kind\":\"frame\",\"frame\":1}}"));
    check(parse_fixture("badesc", &exp) == STUNRUN_EXP_SYNTAX);

    check(write_fixture("nonascii",
        "{\"schema\":\"arcade-experiment/v1\",\"id\":\"x\",\"start\":\"s\","
        "\"events\":[{\"frame\":1,\"port\":\"p\",\"field\":\"f\","
        "\"action\":\"press\",\"label\":\"caf\\u00e9\"}],"
        "\"expect\":{\"kind\":\"frame\",\"frame\":1}}"));
    check(parse_fixture("nonascii", &exp) == STUNRUN_EXP_SYNTAX);

    /* Integer bounds: overflowing values are SYNTAX, negatives are
     * accepted data, and wrong-typed fields are SCHEMA. */
    check(write_fixture("bigint",
        "{\"schema\":\"arcade-experiment/v1\",\"id\":\"x\",\"start\":\"s\","
        "\"events\":[{\"frame\":1,\"port\":\"p\",\"field\":\"f\","
        "\"action\":\"set\",\"value\":9999999999}],"
        "\"expect\":{\"kind\":\"frame\",\"frame\":1}}"));
    check(parse_fixture("bigint", &exp) == STUNRUN_EXP_SYNTAX);

    check(write_fixture("negvalue",
        "{\"schema\":\"arcade-experiment/v1\",\"id\":\"x\",\"start\":\"s\","
        "\"events\":[{\"frame\":1,\"port\":\"p\",\"field\":\"f\","
        "\"action\":\"set\",\"value\":-5}],"
        "\"expect\":{\"kind\":\"frame\",\"frame\":1}}"));
    check(parse_fixture("negvalue", &exp) == STUNRUN_EXP_OK);
    check(exp.events[0].has_value && exp.events[0].value == -5);

    check(write_fixture("wrongtype",
        "{\"schema\":\"arcade-experiment/v1\",\"id\":\"x\",\"start\":\"s\","
        "\"events\":[{\"frame\":\"1\",\"port\":\"p\",\"field\":\"f\","
        "\"action\":\"press\"}],"
        "\"expect\":{\"kind\":\"frame\",\"frame\":1}}"));
    check(parse_fixture("wrongtype", &exp) == STUNRUN_EXP_SCHEMA);

    /* Opaque instrumentation accepts nested objects, arrays, and
     * literals without interpretation. */
    check(write_fixture("nested",
        "{\"schema\":\"arcade-experiment/v1\",\"id\":\"x\",\"start\":\"s\","
        "\"events\":[],\"expect\":{\"kind\":\"frame\",\"frame\":1},"
        "\"instrumentation\":{\"tool\":\"t\",\"window\":\"w\","
        "\"accesses\":[\"r\",\"w\"],\"detail\":{\"ok\":true,"
        "\"flags\":[null,false,7]}}}"));
    check(parse_fixture("nested", &exp) == STUNRUN_EXP_OK);

    /* Resource bounds: oversized files and over-long event lists fail
     * as LIMIT/TOO_BIG instead of overrunning. */
    {
        FILE *fp = fopen("/tmp/stunrun-exp-test-big.json", "wb");
        unsigned i;
        check(fp != NULL);
        if (fp != NULL) {
            for (i = 0; i < 300u * 1024u; i++)
                fputc(' ', fp);
            fclose(fp);
            check(parse_fixture("big", &exp) == STUNRUN_EXP_TOO_BIG);
        }
    }
    {
        FILE *fp = fopen("/tmp/stunrun-exp-test-many.json", "wb");
        unsigned i;
        check(fp != NULL);
        if (fp != NULL) {
            fputs("{\"schema\":\"arcade-experiment/v1\",\"id\":\"x\","
                  "\"start\":\"s\",\"events\":[", fp);
            for (i = 0; i < 1030u; i++) {
                if (i > 0)
                    fputc(',', fp);
                fprintf(fp, "{\"frame\":%u,\"port\":\"p\",\"field\":\"f\","
                            "\"action\":\"press\"}", i);
            }
            fputs("],\"expect\":{\"kind\":\"frame\",\"frame\":1}}", fp);
            fclose(fp);
            check(parse_fixture("many", &exp) == STUNRUN_EXP_LIMIT);
        }
    }

    check(parse_fixture("missing-file-xyz", &exp) == STUNRUN_EXP_IO);
    check(stunrun_experiment_parse(NULL, &exp) == STUNRUN_EXP_IO);
    check(stunrun_experiment_parse("/tmp/stunrun-exp-test-valid.json",
                                   NULL) == STUNRUN_EXP_IO);

    check(strcmp(stunrun_experiment_error_string(STUNRUN_EXP_OK),
                 "ok") == 0);
    check(strcmp(stunrun_experiment_error_string(STUNRUN_EXP_IO),
                 "unreadable file") == 0);
    check(strcmp(stunrun_experiment_error_string(STUNRUN_EXP_TOO_BIG),
                 "file exceeds parser bound") == 0);
    check(strcmp(stunrun_experiment_error_string(STUNRUN_EXP_SYNTAX),
                 "malformed JSON") == 0);
    check(strcmp(stunrun_experiment_error_string(STUNRUN_EXP_SCHEMA),
                 "schema violation") == 0);
    check(strcmp(stunrun_experiment_error_string(STUNRUN_EXP_LIMIT),
                 "event count exceeds parser bound") == 0);

    if (g_failures == 0)
        printf("experiment parser C port: all checks passed\n");
    return g_failures == 0 ? 0 : 1;
}
