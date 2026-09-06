/* See adsp_init_image.h for provenance and confidence notes. */
#include "adsp_init_image.h"

#define STUNRUN_CALL_TO(target) \
    (STUNRUN_ADSP_CALL | ((uint32_t)(target) << 4) | STUNRUN_ADSP_COND_ALWAYS)
#define STUNRUN_JUMP_TO(target) \
    (STUNRUN_ADSP_JUMP | ((uint32_t)(target) << 4) | STUNRUN_ADSP_COND_ALWAYS)

/* Literal decoded setup words 0x0006..0x003C from the original image. */
static const uint32_t kSetup[] = {
    0x00340008u, 0x00340009u, 0x0034000Au, 0x0034000Bu,
    0x00380008u, 0x00380009u, 0x0038000Au, 0x0038000Bu,
    0x00340015u, 0x00380017u, 0x0040000Au, 0x0090030Au,
    0x0091FE3Au, 0x0081FFFAu, 0x0023620Fu, 0x00180130u,
    0x004FFFFAu, 0x0091FFFAu, 0x0034A000u, 0x009401B0u,
    0x003C0007u, 0x0035FEF0u, 0x00390190u, 0x003C0065u,
    0x0014020Eu, 0x006000A1u, 0x005800A3u, 0x0035FF50u,
    0x00390000u, 0x003C0095u, 0x0014026Eu, 0x006000A1u,
    0x005800A3u, 0x0035FE90u, 0x00390090u, 0x003C0065u,
    0x001402CEu, 0x006000A1u, 0x005800A3u, 0x0081FFEAu,
    0x0092007Au, 0x004007CAu, 0x0090029Au, 0x0040001Au,
    0x009001EAu, 0x0040000Au, 0x0092003Au, 0x0092002Au,
    0x009002BAu, 0x009002AAu, 0x0090108Au, 0x0092002Au,
    0x0090958Au, 0x009002DAu, 0x009002EAu,
};

/* Mailbox prelude words 0x0040..0x004F. */
static const uint32_t kMailbox[] = {
    0x00800304u, 0x0022200Fu, 0x0090030Au, 0x008801B1u,
    0x00380175u, 0x00090015u, 0x007000A7u, 0x00440004u,
    0x0023820Fu, 0x00180500u, 0x0080030Au, 0x0091FE3Au,
    0x00920060u, 0x0081FE3Au, 0x00227A0Fu, 0x001804D1u,
};

/* Bounded decoded body at 0x0780..0x07A1. */
static const uint32_t kRoutine0780[] = {
    0x00380014u, 0x00380035u, 0x00340014u, 0x00340037u,
    0x00380008u, 0x00340009u, 0x004FFFF0u, 0x0047FFF1u,
    0x00412485u, 0x004123C4u, 0x0041236Au, 0x00412422u,
    0x00400003u, 0x00480006u, 0x00392360u, 0x00580000u,
    0x00580010u, 0x00580050u, 0x00580041u, 0x00580000u,
    0x00580010u, 0x005800A0u, 0x00580021u, 0x00989550u,
    0x00580030u, 0x00580060u, 0x00580040u, 0x00580051u,
    0x00580030u, 0x00580060u, 0x00580020u, 0x005800A1u,
    0x00989560u, 0x000A000Fu,
};

/* Bounded decoded body at 0x0834..0x0846. */
static const uint32_t kRoutine0834[] = {
    0x00340014u, 0x00340087u, 0x00340009u, 0x00340008u,
    0x00349590u, 0x003495A1u, 0x00AFFFF7u, 0x00AFFFF7u,
    0x00AFFFF7u, 0x00AFFFF7u, 0x00AFFFF7u, 0x00AFFFF7u,
    0x00A7FFF3u, 0x00A7FFF3u, 0x00A7FFF3u, 0x00A7FFF3u,
    0x00A7FFF3u, 0x00A7FFF3u, 0x000A000Fu,
};

static const stunrun_adsp_dm_landmark_t kLandmarks[] = {
    { 0x0955u, 0x1242u },
    { 0x0956u, 0x124Eu },
    { 0x0959u, 0x7FFFu },
    { 0x095Au, 0xFFFFu },
};

size_t stunrun_adsp_fixture_words(stunrun_adsp_fixture_t fixture)
{
    switch (fixture) {
    case STUNRUN_ADSP_FIXTURE_NOP:
        return STUNRUN_ADSP_NOP_WORDS;
    case STUNRUN_ADSP_FIXTURE_RESET_LOOP:
        return STUNRUN_ADSP_RESET_LOOP_WORDS;
    case STUNRUN_ADSP_FIXTURE_INIT_PREFIX:
        return STUNRUN_ADSP_INIT_PREFIX_WORDS;
    case STUNRUN_ADSP_FIXTURE_INIT_STATE:
        return STUNRUN_ADSP_INIT_STATE_WORDS;
    default:
        return 0;
    }
}

static void emit_init_prefix(uint32_t *out)
{
    out[STUNRUN_ADSP_ENTRY] = STUNRUN_CALL_TO(STUNRUN_ADSP_SUB_0780);
    out[0x0005u] = STUNRUN_CALL_TO(STUNRUN_ADSP_SUB_0834);
    out[0x0006u] = STUNRUN_JUMP_TO(0x0006u);
    out[STUNRUN_ADSP_SUB_0780] = STUNRUN_ADSP_RTS;
    out[STUNRUN_ADSP_SUB_0834] = STUNRUN_ADSP_RTS;
}

static void emit_init_state(uint32_t *out)
{
    size_t i;
    emit_init_prefix(out);
    for (i = 0; i < sizeof(kSetup) / sizeof(kSetup[0]); i++)
        out[0x0006u + i] = kSetup[i];
    out[0x003Du] = STUNRUN_CALL_TO(STUNRUN_ADSP_SUB_0780);
    out[0x003Eu] = STUNRUN_CALL_TO(STUNRUN_ADSP_SUB_0834);
    out[0x003Fu] = STUNRUN_JUMP_TO(0x0043u);
    for (i = 0; i < sizeof(kMailbox) / sizeof(kMailbox[0]); i++)
        out[0x0040u + i] = kMailbox[i];
    out[STUNRUN_ADSP_MAILBOX_BOUNDARY] = STUNRUN_JUMP_TO(0x0050u);
    for (i = 0; i < sizeof(kRoutine0780) / sizeof(kRoutine0780[0]); i++)
        out[STUNRUN_ADSP_SUB_0780 + i] = kRoutine0780[i];
    for (i = 0; i < sizeof(kRoutine0834) / sizeof(kRoutine0834[0]); i++)
        out[STUNRUN_ADSP_SUB_0834 + i] = kRoutine0834[i];
}

size_t stunrun_adsp_emit(stunrun_adsp_fixture_t fixture,
                         uint32_t *out, size_t count)
{
    size_t need = stunrun_adsp_fixture_words(fixture);
    size_t i;
    if (need == 0 || out == NULL)
        return 0;
    if (count > need)
        count = need;
    for (i = 0; i < count; i++)
        out[i] = 0;
    switch (fixture) {
    case STUNRUN_ADSP_FIXTURE_NOP:
        break;
    case STUNRUN_ADSP_FIXTURE_RESET_LOOP:
        if (count > STUNRUN_ADSP_ENTRY)
            out[STUNRUN_ADSP_ENTRY] = STUNRUN_JUMP_TO(STUNRUN_ADSP_ENTRY);
        break;
    case STUNRUN_ADSP_FIXTURE_INIT_PREFIX:
        if (count > STUNRUN_ADSP_ENTRY) {
            /* Emit into a full-size scratch then copy the prefix so short
             * reads still observe the entry words first. */
            static uint32_t scratch[STUNRUN_ADSP_INIT_PREFIX_WORDS];
            size_t j;
            for (j = 0; j < STUNRUN_ADSP_INIT_PREFIX_WORDS; j++)
                scratch[j] = 0;
            emit_init_prefix(scratch);
            for (i = 0; i < count; i++)
                out[i] = scratch[i];
        }
        break;
    case STUNRUN_ADSP_FIXTURE_INIT_STATE:
        if (count > STUNRUN_ADSP_ENTRY) {
            static uint32_t scratch[STUNRUN_ADSP_INIT_STATE_WORDS];
            size_t j;
            for (j = 0; j < STUNRUN_ADSP_INIT_STATE_WORDS; j++)
                scratch[j] = 0;
            emit_init_state(scratch);
            for (i = 0; i < count; i++)
                out[i] = scratch[i];
        }
        break;
    default:
        return 0;
    }
    return count;
}

const stunrun_adsp_dm_landmark_t *stunrun_adsp_dm_landmarks(void)
{
    return kLandmarks;
}

void stunrun_adsp_apply_dm_landmarks(uint16_t dm[STUNRUN_ADSP_DM_SIZE])
{
    size_t i;
    if (dm == NULL)
        return;
    for (i = 0; i < STUNRUN_ADSP_DM_LANDMARK_COUNT; i++)
        dm[kLandmarks[i].addr] = kLandmarks[i].value;
}
