#include "text_cursor.h"

#include <stdio.h>

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", #condition); \
        failures++; \
    } \
} while (0)

int main(void)
{
    static const uint16_t time_words[] = {0x3A30u, 0x3533u, 0x302Eu, 0x0000u};
    static const uint16_t credits_words[] = {
        0x7243u, 0x6465u, 0x7469u, 0x3A73u, 0x3020u, 0x0020u
    };
    stunrun_gsp_text_glyph_t glyphs[16];
    size_t count;

    count = stunrun_gsp_text_cursor_decode(
        0xFFFEA4C0u, 0x00FD0040u, 0x28,
        time_words, sizeof(time_words) / sizeof(*time_words),
        glyphs, sizeof(glyphs) / sizeof(*glyphs));
    CHECK(count == 6u);
    CHECK(glyphs[0].descriptor_word_address == 0xFFFEA4C0u);
    CHECK(glyphs[0].a0_after == 0xFFFEA4C8u);
    CHECK(glyphs[0].a1 == 0x00FD0040u);
    CHECK(glyphs[0].lane == 0u && glyphs[0].glyph_code == '0');
    CHECK(glyphs[0].x == 64 && glyphs[0].y == 213);
    CHECK(glyphs[1].descriptor_word_address == 0xFFFEA4C0u);
    CHECK(glyphs[1].a0_after == 0xFFFEA4D0u);
    CHECK(glyphs[1].a1 == 0x00FD0048u);
    CHECK(glyphs[1].lane == 1u && glyphs[1].glyph_code == ':');
    CHECK(glyphs[5].a0_after == 0xFFFEA4F0u);
    CHECK(glyphs[5].x == 104 && glyphs[5].glyph_code == '0');

    count = stunrun_gsp_text_cursor_decode(
        0xFFFEA810u, 0x010800D4u, 0x28,
        credits_words, sizeof(credits_words) / sizeof(*credits_words),
        glyphs, sizeof(glyphs) / sizeof(*glyphs));
    CHECK(count == 11u);
    CHECK(glyphs[0].a0_after == 0xFFFEA818u && glyphs[0].x == 212);
    CHECK(glyphs[0].glyph_code == 'C' && glyphs[0].lane == 0u);
    CHECK(glyphs[1].a0_after == 0xFFFEA820u && glyphs[1].x == 220);
    CHECK(glyphs[1].glyph_code == 'r' && glyphs[1].lane == 1u);
    CHECK(glyphs[10].glyph_code == ' ' && glyphs[10].x == 292);

    CHECK(stunrun_gsp_text_cursor_decode(
        0u, 0u, 0, time_words, 1u, glyphs, 0u) == 0u);
    if (failures != 0)
        return 1;
    puts("gsp-text-cursor-slice: PASS");
    return 0;
}
