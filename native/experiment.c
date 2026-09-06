/* See experiment.h for the contract. */
#include "experiment.h"

#include <stdio.h>
#include <string.h>

typedef struct stunrun_exp_cursor {
    const char *p;
    const char *end;
} stunrun_exp_cursor_t;

static void skip_ws(stunrun_exp_cursor_t *c)
{
    while (c->p < c->end &&
           (*c->p == ' ' || *c->p == '\t' ||
            *c->p == '\n' || *c->p == '\r'))
        c->p++;
}

static int take(stunrun_exp_cursor_t *c, char want)
{
    skip_ws(c);
    if (c->p < c->end && *c->p == want) {
        c->p++;
        return 1;
    }
    return 0;
}

/* Lead-character triage for a typed value: 1 when one of leads is next
 * (caller parses; parse failure is SYNTAX), 0 at end of input (SYNTAX),
 * -1 for any other character (wrong JSON type: SCHEMA). */
static int check_lead(stunrun_exp_cursor_t *c, const char *leads)
{
    skip_ws(c);
    if (c->p >= c->end)
        return 0;
    return strchr(leads, *c->p) != NULL ? 1 : -1;
}

#define INT_LEADS "-0123456789"

/* Parse a JSON string into buf (always NUL-terminated). Handles the common
 * escapes; \u is accepted only for code points U+0000-U+007F. */
static int parse_string(stunrun_exp_cursor_t *c, char *buf, size_t cap)
{
    size_t len = 0;
    unsigned code;
    int i;
    skip_ws(c);
    if (c->p >= c->end || *c->p != '"')
        return 0;
    c->p++;
    while (c->p < c->end && *c->p != '"') {
        char ch = *c->p;
        if (ch == '\\') {
            c->p++;
            if (c->p >= c->end)
                return 0;
            switch (*c->p) {
            case '"': ch = '"'; break;
            case '\\': ch = '\\'; break;
            case '/': ch = '/'; break;
            case 'b': ch = '\b'; break;
            case 'f': ch = '\f'; break;
            case 'n': ch = '\n'; break;
            case 'r': ch = '\r'; break;
            case 't': ch = '\t'; break;
            case 'u':
                code = 0;
                for (i = 0; i < 4; i++) {
                    char h;
                    c->p++;
                    if (c->p >= c->end)
                        return 0;
                    h = *c->p;
                    code <<= 4;
                    if (h >= '0' && h <= '9')
                        code |= (unsigned)(h - '0');
                    else if (h >= 'a' && h <= 'f')
                        code |= (unsigned)(h - 'a' + 10);
                    else if (h >= 'A' && h <= 'F')
                        code |= (unsigned)(h - 'A' + 10);
                    else
                        return 0;
                }
                if (code > 0x7Fu)
                    return 0;
                ch = (char)code;
                break;
            default:
                return 0;
            }
            c->p++;
        } else {
            if ((unsigned char)ch < 0x20u)
                return 0;
            c->p++;
        }
        if (len + 1 >= cap)
            return 0;
        buf[len++] = ch;
    }
    if (c->p >= c->end || *c->p != '"')
        return 0;
    c->p++;
    buf[len] = '\0';
    return 1;
}

/* Strict JSON integer into *out (no fractions/exponents). */
static int parse_int(stunrun_exp_cursor_t *c, long *out)
{
    int neg = 0;
    long value = 0;
    int digits = 0;
    skip_ws(c);
    if (c->p < c->end && *c->p == '-') {
        neg = 1;
        c->p++;
    }
    while (c->p < c->end && *c->p >= '0' && *c->p <= '9') {
        value = value * 10 + (*c->p - '0');
        if (value > 1000000000L)
            return 0;
        c->p++;
        digits++;
    }
    if (digits == 0)
        return 0;
    *out = neg ? -value : value;
    return 1;
}

/* String field: wrong type -> SCHEMA, cut-off content -> SYNTAX. */
static stunrun_exp_error_t parse_text_field(stunrun_exp_cursor_t *c,
                                            char *buf, size_t cap,
                                            int allow_empty)
{
    int lead = check_lead(c, "\"");
    if (lead == 0)
        return STUNRUN_EXP_SYNTAX;
    if (lead < 0)
        return STUNRUN_EXP_SCHEMA;
    if (!parse_string(c, buf, cap))
        return STUNRUN_EXP_SYNTAX;
    if (!allow_empty && buf[0] == '\0')
        return STUNRUN_EXP_SCHEMA;
    return STUNRUN_EXP_OK;
}

/* Integer field with range check: wrong type -> SCHEMA, cut-off content
 * or out-of-range value -> SYNTAX/SCHEMA respectively. */
static stunrun_exp_error_t parse_int_field(stunrun_exp_cursor_t *c,
                                           long lo, long hi, long *out)
{
    int lead = check_lead(c, INT_LEADS);
    long value;
    if (lead == 0)
        return STUNRUN_EXP_SYNTAX;
    if (lead < 0)
        return STUNRUN_EXP_SCHEMA;
    if (!parse_int(c, &value))
        return STUNRUN_EXP_SYNTAX;
    if (value < lo || value > hi)
        return STUNRUN_EXP_SCHEMA;
    *out = value;
    return STUNRUN_EXP_OK;
}

static int parse_literal(stunrun_exp_cursor_t *c, const char *word)
{
    size_t n = strlen(word);
    skip_ws(c);
    if ((size_t)(c->end - c->p) < n || memcmp(c->p, word, n) != 0)
        return 0;
    c->p += n;
    return 1;
}

/* Skip any JSON value (used for the opaque instrumentation subtree). */
static int skip_value(stunrun_exp_cursor_t *c)
{
    skip_ws(c);
    if (c->p >= c->end)
        return 0;
    if (*c->p == '{') {
        int first = 1;
        c->p++;
        skip_ws(c);
        if (take(c, '}'))
            return 1;
        for (;;) {
            char key[STUNRUN_EXP_STR_LEN];
            if (!first && !take(c, ','))
                return 0;
            first = 0;
            if (!parse_string(c, key, sizeof(key)))
                return 0;
            if (!take(c, ':') || !skip_value(c))
                return 0;
            skip_ws(c);
            if (take(c, '}'))
                return 1;
        }
    }
    if (*c->p == '[') {
        int first = 1;
        c->p++;
        skip_ws(c);
        if (take(c, ']'))
            return 1;
        for (;;) {
            if (!first && !take(c, ','))
                return 0;
            first = 0;
            if (!skip_value(c))
                return 0;
            skip_ws(c);
            if (take(c, ']'))
                return 1;
        }
    }
    if (*c->p == '"') {
        char tmp[8];
        return parse_string(c, tmp, sizeof(tmp));
    }
    if ((*c->p >= '0' && *c->p <= '9') || *c->p == '-') {
        long ignored;
        return parse_int(c, &ignored);
    }
    return parse_literal(c, "true") || parse_literal(c, "false") ||
           parse_literal(c, "null");
}

static int id_char_ok(char ch, int first)
{
    if (ch >= 'a' && ch <= 'z')
        return 1;
    if (ch >= '0' && ch <= '9')
        return !first;
    return ch == '_' || ch == '-';
}

static int id_ok(const char *id)
{
    size_t i;
    if (id[0] == '\0')
        return 0;
    for (i = 0; id[i] != '\0'; i++) {
        if (!id_char_ok(id[i], i == 0))
            return 0;
    }
    return 1;
}

static int parse_action(const char *word, stunrun_exp_action_t *out)
{
    if (strcmp(word, "press") == 0) {
        *out = STUNRUN_EXP_PRESS;
        return 1;
    }
    if (strcmp(word, "release") == 0) {
        *out = STUNRUN_EXP_RELEASE;
        return 1;
    }
    if (strcmp(word, "set") == 0) {
        *out = STUNRUN_EXP_SET;
        return 1;
    }
    return 0;
}

/* Parse one event object. */
static stunrun_exp_error_t parse_event(stunrun_exp_cursor_t *c,
                                       stunrun_exp_event_t *ev)
{
    char key[STUNRUN_EXP_STR_LEN];
    char sval[STUNRUN_EXP_STR_LEN];
    stunrun_exp_error_t err;
    long ival;
    int first = 1;
    int has_frame = 0, has_port = 0, has_field = 0, has_action = 0;
    int lead = check_lead(c, "{");
    if (lead == 0)
        return STUNRUN_EXP_SYNTAX;
    if (lead < 0)
        return STUNRUN_EXP_SCHEMA;
    c->p++;
    ev->port[0] = '\0';
    ev->field[0] = '\0';
    ev->label[0] = '\0';
    ev->has_value = 0;
    ev->value = 0;
    ev->frame = 0;
    skip_ws(c);
    if (take(c, '}'))
        return STUNRUN_EXP_SCHEMA;
    for (;;) {
        if (!first && !take(c, ','))
            return STUNRUN_EXP_SYNTAX;
        first = 0;
        if (!parse_string(c, key, sizeof(key)))
            return STUNRUN_EXP_SYNTAX;
        if (!take(c, ':'))
            return STUNRUN_EXP_SYNTAX;
        if (strcmp(key, "frame") == 0) {
            err = parse_int_field(c, 0, STUNRUN_EXP_MAX_TERMINAL_FRAME,
                                  &ival);
            if (err != STUNRUN_EXP_OK)
                return err;
            ev->frame = (unsigned)ival;
            has_frame = 1;
        } else if (strcmp(key, "port") == 0) {
            err = parse_text_field(c, ev->port, sizeof(ev->port), 0);
            if (err != STUNRUN_EXP_OK)
                return err;
            has_port = 1;
        } else if (strcmp(key, "field") == 0) {
            err = parse_text_field(c, ev->field, sizeof(ev->field), 0);
            if (err != STUNRUN_EXP_OK)
                return err;
            has_field = 1;
        } else if (strcmp(key, "action") == 0) {
            err = parse_text_field(c, sval, sizeof(sval), 0);
            if (err != STUNRUN_EXP_OK)
                return err;
            if (!parse_action(sval, &ev->action))
                return STUNRUN_EXP_SCHEMA;
            has_action = 1;
        } else if (strcmp(key, "value") == 0) {
            err = parse_int_field(c, -1000000000L, 1000000000L, &ival);
            if (err != STUNRUN_EXP_OK)
                return err;
            ev->value = (int)ival;
            ev->has_value = 1;
        } else if (strcmp(key, "label") == 0) {
            err = parse_text_field(c, ev->label, sizeof(ev->label), 1);
            if (err != STUNRUN_EXP_OK)
                return err;
        } else {
            return STUNRUN_EXP_SCHEMA;
        }
        skip_ws(c);
        if (take(c, '}'))
            break;
    }
    if (!has_frame || !has_port || !has_field || !has_action)
        return STUNRUN_EXP_SCHEMA;
    if (ev->action == STUNRUN_EXP_SET && !ev->has_value)
        return STUNRUN_EXP_SCHEMA;
    return STUNRUN_EXP_OK;
}

/* Parse the expect object. */
static stunrun_exp_error_t parse_expect(stunrun_exp_cursor_t *c,
                                        stunrun_experiment_t *exp)
{
    char key[STUNRUN_EXP_STR_LEN];
    char sval[STUNRUN_EXP_STR_LEN];
    stunrun_exp_error_t err;
    long ival;
    int first = 1;
    int has_kind = 0, has_frame = 0, has_terminal = 0, frame_kind = 0;
    int lead = check_lead(c, "{");
    if (lead == 0)
        return STUNRUN_EXP_SYNTAX;
    if (lead < 0)
        return STUNRUN_EXP_SCHEMA;
    c->p++;
    exp->terminal_frame = 0;
    exp->has_timeout_frames = 0;
    exp->timeout_frames = 0;
    exp->terminal[0] = '\0';
    skip_ws(c);
    if (take(c, '}'))
        return STUNRUN_EXP_SCHEMA;
    for (;;) {
        if (!first && !take(c, ','))
            return STUNRUN_EXP_SYNTAX;
        first = 0;
        if (!parse_string(c, key, sizeof(key)))
            return STUNRUN_EXP_SYNTAX;
        if (!take(c, ':'))
            return STUNRUN_EXP_SYNTAX;
        if (strcmp(key, "kind") == 0) {
            err = parse_text_field(c, sval, sizeof(sval), 0);
            if (err != STUNRUN_EXP_OK)
                return err;
            if (strcmp(sval, "frame") == 0)
                frame_kind = 1;
            else if (strcmp(sval, "bounded_observation") == 0)
                frame_kind = 0;
            else
                return STUNRUN_EXP_SCHEMA;
            has_kind = 1;
        } else if (strcmp(key, "frame") == 0) {
            err = parse_int_field(c, 0, STUNRUN_EXP_MAX_TERMINAL_FRAME,
                                  &ival);
            if (err != STUNRUN_EXP_OK)
                return err;
            exp->terminal_frame = (unsigned)ival;
            has_frame = 1;
        } else if (strcmp(key, "timeout_frames") == 0) {
            err = parse_int_field(c, 1, STUNRUN_EXP_MAX_TERMINAL_FRAME,
                                  &ival);
            if (err != STUNRUN_EXP_OK)
                return err;
            exp->timeout_frames = (unsigned)ival;
            exp->has_timeout_frames = 1;
        } else if (strcmp(key, "terminal") == 0) {
            err = parse_text_field(c, exp->terminal, sizeof(exp->terminal),
                                   0);
            if (err != STUNRUN_EXP_OK)
                return err;
            has_terminal = 1;
        } else {
            return STUNRUN_EXP_SCHEMA;
        }
        skip_ws(c);
        if (take(c, '}'))
            break;
    }
    if (!has_kind)
        return STUNRUN_EXP_SCHEMA;
    if (frame_kind) {
        if (!has_frame || has_terminal)
            return STUNRUN_EXP_SCHEMA;
        exp->expect_kind = STUNRUN_EXP_EXPECT_FRAME;
    } else {
        if (!has_terminal || has_frame || exp->has_timeout_frames)
            return STUNRUN_EXP_SCHEMA;
        exp->expect_kind = STUNRUN_EXP_EXPECT_BOUNDED_OBSERVATION;
    }
    return STUNRUN_EXP_OK;
}

stunrun_exp_error_t stunrun_experiment_parse(const char *path,
                                             stunrun_experiment_t *out)
{
    static char text[STUNRUN_EXP_MAX_FILE_BYTES + 1];
    FILE *fp;
    size_t got;
    stunrun_exp_cursor_t c;
    stunrun_exp_error_t err;
    char key[STUNRUN_EXP_STR_LEN];
    char sval[STUNRUN_EXP_STR_LEN];
    int first = 1;
    int lead;
    int has_schema = 0, has_id = 0, has_start = 0, has_events = 0,
        has_expect = 0;
    if (path == NULL || out == NULL)
        return STUNRUN_EXP_IO;
    memset(out, 0, sizeof(*out));
    fp = fopen(path, "rb");
    if (fp == NULL)
        return STUNRUN_EXP_IO;
    got = fread(text, 1, sizeof(text) - 1, fp);
    if (!feof(fp)) {
        fclose(fp);
        return STUNRUN_EXP_TOO_BIG;
    }
    fclose(fp);
    text[got] = '\0';
    c.p = text;
    c.end = text + got;
    lead = check_lead(&c, "{");
    if (lead == 0)
        return STUNRUN_EXP_SYNTAX;
    if (lead < 0)
        return STUNRUN_EXP_SCHEMA;
    c.p++;
    skip_ws(&c);
    if (take(&c, '}'))
        return STUNRUN_EXP_SCHEMA;
    for (;;) {
        if (!first && !take(&c, ','))
            return STUNRUN_EXP_SYNTAX;
        first = 0;
        if (!parse_string(&c, key, sizeof(key)))
            return STUNRUN_EXP_SYNTAX;
        if (!take(&c, ':'))
            return STUNRUN_EXP_SYNTAX;
        if (strcmp(key, "schema") == 0) {
            err = parse_text_field(&c, sval, sizeof(sval), 0);
            if (err != STUNRUN_EXP_OK)
                return err;
            if (strcmp(sval, "arcade-experiment/v1") != 0)
                return STUNRUN_EXP_SCHEMA;
            has_schema = 1;
        } else if (strcmp(key, "id") == 0) {
            err = parse_text_field(&c, out->id, sizeof(out->id), 0);
            if (err != STUNRUN_EXP_OK)
                return err;
            if (!id_ok(out->id))
                return STUNRUN_EXP_SCHEMA;
            has_id = 1;
        } else if (strcmp(key, "start") == 0) {
            err = parse_text_field(&c, out->start, sizeof(out->start), 0);
            if (err != STUNRUN_EXP_OK)
                return err;
            has_start = 1;
        } else if (strcmp(key, "events") == 0) {
            int efirst = 1;
            int elead = check_lead(&c, "[");
            if (elead == 0)
                return STUNRUN_EXP_SYNTAX;
            if (elead < 0)
                return STUNRUN_EXP_SCHEMA;
            c.p++;
            out->event_count = 0;
            skip_ws(&c);
            if (take(&c, ']')) {
                has_events = 1;
            } else {
                for (;;) {
                    if (!efirst && !take(&c, ','))
                        return STUNRUN_EXP_SYNTAX;
                    efirst = 0;
                    if (out->event_count >= STUNRUN_EXP_MAX_EVENTS)
                        return STUNRUN_EXP_LIMIT;
                    err = parse_event(
                        &c, &out->events[out->event_count]);
                    if (err != STUNRUN_EXP_OK)
                        return err;
                    out->event_count++;
                    skip_ws(&c);
                    if (take(&c, ']'))
                        break;
                }
                has_events = 1;
            }
        } else if (strcmp(key, "expect") == 0) {
            err = parse_expect(&c, out);
            if (err != STUNRUN_EXP_OK)
                return err;
            has_expect = 1;
        } else if (strcmp(key, "instrumentation") == 0) {
            if (!skip_value(&c))
                return STUNRUN_EXP_SYNTAX;
        } else {
            return STUNRUN_EXP_SCHEMA;
        }
        skip_ws(&c);
        if (take(&c, '}'))
            break;
    }
    skip_ws(&c);
    if (c.p != c.end)
        return STUNRUN_EXP_SYNTAX;
    if (!has_schema || !has_id || !has_start || !has_events || !has_expect)
        return STUNRUN_EXP_SCHEMA;
    return STUNRUN_EXP_OK;
}

const char *stunrun_experiment_error_string(stunrun_exp_error_t error)
{
    switch (error) {
    case STUNRUN_EXP_OK:
        return "ok";
    case STUNRUN_EXP_IO:
        return "unreadable file";
    case STUNRUN_EXP_TOO_BIG:
        return "file exceeds parser bound";
    case STUNRUN_EXP_SYNTAX:
        return "malformed JSON";
    case STUNRUN_EXP_SCHEMA:
        return "schema violation";
    case STUNRUN_EXP_LIMIT:
        return "event count exceeds parser bound";
    default:
        return "unknown";
    }
}
