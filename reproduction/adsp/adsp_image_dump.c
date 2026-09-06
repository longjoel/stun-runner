/* Emit a fixed-origin ADSP image from the shared C implementation.
 *
 * This small ROM-free utility is the runtime bridge for the C port: the
 * resulting big-endian word image has the same byte layout consumed by the
 * MAME replacement loader.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "adsp_init_image.h"

static int fixture_from_name(const char *name, stunrun_adsp_fixture_t *out)
{
    if (strcmp(name, "nop") == 0)
        *out = STUNRUN_ADSP_FIXTURE_NOP;
    else if (strcmp(name, "reset-loop") == 0)
        *out = STUNRUN_ADSP_FIXTURE_RESET_LOOP;
    else if (strcmp(name, "init-prefix") == 0)
        *out = STUNRUN_ADSP_FIXTURE_INIT_PREFIX;
    else if (strcmp(name, "init-state") == 0)
        *out = STUNRUN_ADSP_FIXTURE_INIT_STATE;
    else
        return 0;
    return 1;
}

int main(int argc, char **argv)
{
    static uint32_t words[STUNRUN_ADSP_INIT_STATE_WORDS];
    stunrun_adsp_fixture_t fixture;
    size_t count;
    size_t i;
    FILE *out;

    if (argc != 3 || !fixture_from_name(argv[1], &fixture)) {
        fprintf(stderr, "usage: %s {nop|reset-loop|init-prefix|init-state} OUTPUT\n",
                argv[0]);
        return 2;
    }
    count = stunrun_adsp_fixture_words(fixture);
    if (count == 0 || stunrun_adsp_emit(fixture, words, count) != count)
        return 1;
    out = fopen(argv[2], "wb");
    if (out == NULL) {
        perror(argv[2]);
        return 1;
    }
    for (i = 0; i < count; i++) {
        unsigned char bytes[4] = {
            (unsigned char)(words[i] >> 24),
            (unsigned char)(words[i] >> 16),
            (unsigned char)(words[i] >> 8),
            (unsigned char)words[i]
        };
        if (fwrite(bytes, 1, sizeof(bytes), out) != sizeof(bytes)) {
            perror(argv[2]);
            fclose(out);
            return 1;
        }
    }
    if (fclose(out) != 0) {
        perror(argv[2]);
        return 1;
    }
    return 0;
}
