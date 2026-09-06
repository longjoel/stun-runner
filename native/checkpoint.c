/* See checkpoint.h for the contract. */
#include "checkpoint.h"

#include <stdio.h>
#include <string.h>

typedef struct stunrun_emit_writer {
    char *out;
    size_t capacity;
    size_t pos;
    int overflow;
} stunrun_emit_writer_t;

static void emit_raw(stunrun_emit_writer_t *w, const char *text)
{
    size_t n;
    if (w->overflow)
        return;
    n = strlen(text);
    if (w->pos + n >= w->capacity) {
        w->overflow = 1;
        return;
    }
    memcpy(w->out + w->pos, text, n);
    w->pos += n;
}

static void emit_u32(stunrun_emit_writer_t *w, uint32_t value)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%u", (unsigned)value);
    emit_raw(w, buf);
}

static void emit_str(stunrun_emit_writer_t *w, const char *value)
{
    /* Values are fixed provenance-controlled ASCII (tags, versions);
     * escaping is out of scope and asserted safe by construction. */
    emit_raw(w, "\"");
    emit_raw(w, value);
    emit_raw(w, "\"");
}

size_t stunrun_checkpoint_emit(const stunrun_checkpoint_t *cp,
                               char *out, size_t capacity)
{
    stunrun_emit_writer_t w;
    char num[32];
    unsigned i;
    if (cp == NULL || out == NULL || capacity == 0)
        return 0;
    w.out = out;
    w.capacity = capacity;
    w.pos = 0;
    w.overflow = 0;
    emit_raw(&w, "{\n");
    emit_raw(&w, "  \"schema\": ");
    emit_str(&w, STUNRUN_CHECKPOINT_SCHEMA);
    emit_raw(&w, ",\n  \"system\": ");
    emit_str(&w, cp->system);
    emit_raw(&w, ",\n  \"description\": ");
    emit_str(&w, cp->description);
    emit_raw(&w, ",\n  \"mame\": ");
    emit_str(&w, cp->mame);
    emit_raw(&w, ",\n");
    snprintf(num, sizeof(num), "  \"frame\": %u,\n", cp->frame);
    emit_raw(&w, num);
    snprintf(num, sizeof(num), "  \"time_seconds\": %.5f,\n",
             cp->time_seconds);
    emit_raw(&w, num);
    emit_raw(&w, "  \"processors\": {\n    \"maincpu\": {\"tag\": ");
    emit_str(&w, cp->maincpu_tag);
    emit_raw(&w, ", \"pc\": ");
    emit_u32(&w, cp->maincpu_pc);
    emit_raw(&w, ", \"sr\": ");
    emit_u32(&w, cp->maincpu_sr);
    emit_raw(&w, ", \"sp\": ");
    emit_u32(&w, cp->maincpu_sp);
    emit_raw(&w, "},\n    \"gsp\": {\"tag\": ");
    emit_str(&w, cp->gsp_tag);
    emit_raw(&w, ", \"pc\": ");
    emit_u32(&w, cp->gsp_pc);
    emit_raw(&w, ", \"st\": ");
    emit_u32(&w, cp->gsp_st);
    emit_raw(&w, "},\n    \"adsp\": {\"tag\": ");
    emit_str(&w, cp->adsp_tag);
    emit_raw(&w, ", \"pc\": ");
    emit_u32(&w, cp->adsp_pc);
    emit_raw(&w, ", \"astat\": ");
    emit_u32(&w, cp->adsp_astat);
    emit_raw(&w, "},\n    \"soundcpu\": {\"tag\": ");
    emit_str(&w, cp->soundcpu_tag);
    emit_raw(&w, ", \"pc\": ");
    emit_u32(&w, cp->soundcpu_pc);
    emit_raw(&w, ", \"p\": ");
    emit_u32(&w, cp->soundcpu_p);
    emit_raw(&w, "}\n  },\n  \"regions\": {\n    \"adsp_program\": {\n");
    emit_raw(&w, "      \"space\": ");
    emit_str(&w, cp->region_space);
    emit_raw(&w, ",\n      \"range\": ");
    emit_str(&w, cp->region_range);
    emit_raw(&w, ",\n");
    snprintf(num, sizeof(num), "      \"word_width\": %u,\n",
             cp->region_word_width);
    emit_raw(&w, num);
    snprintf(num, sizeof(num), "      \"nonzero_words\": %u,\n",
             cp->region_nonzero_words);
    emit_raw(&w, num);
    emit_raw(&w, "      \"sum32\": ");
    emit_u32(&w, cp->region_sum32);
    emit_raw(&w, ",\n      \"first_nonzero_words\": [");
    for (i = 0; i < STUNRUN_CHECKPOINT_FIRST_WORDS; i++) {
        if (i > 0)
            emit_raw(&w, ", ");
        emit_u32(&w, cp->region_first_words[i]);
    }
    emit_raw(&w, "]\n    }\n  },\n  \"selectors\": "
                "{\"adsp_program_loaded\": ");
    emit_raw(&w, cp->adsp_program_loaded ? "true" : "false");
    emit_raw(&w, "}\n}\n");
    if (w.overflow)
        return 0;
    w.out[w.pos] = '\0';
    return w.pos;
}
