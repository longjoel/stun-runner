/* See render.h for the deliberately small M4 rendering contract. */
#include "render.h"

#include <stdio.h>
#include <string.h>

void stunrun_render_init(stunrun_renderer_t *renderer)
{
    if (renderer == NULL)
        return;
    memset(renderer, 0, sizeof(*renderer));
}

void stunrun_render_begin(stunrun_renderer_t *renderer, unsigned frame,
                          uint8_t red, uint8_t green, uint8_t blue)
{
    size_t i;
    if (renderer == NULL)
        return;
    renderer->frame = frame;
    for (i = 0; i < STUNRUN_RENDER_BYTES; i += 3u) {
        renderer->pixels[i] = red;
        renderer->pixels[i + 1u] = green;
        renderer->pixels[i + 2u] = blue;
    }
}

int stunrun_render_set_pixel(stunrun_renderer_t *renderer, unsigned x,
                             unsigned y, uint8_t red, uint8_t green,
                             uint8_t blue)
{
    size_t offset;
    if (renderer == NULL || x >= STUNRUN_RENDER_WIDTH ||
        y >= STUNRUN_RENDER_HEIGHT)
        return 0;
    offset = ((size_t)y * STUNRUN_RENDER_WIDTH + x) * 3u;
    renderer->pixels[offset] = red;
    renderer->pixels[offset + 1u] = green;
    renderer->pixels[offset + 2u] = blue;
    return 1;
}

int stunrun_render_blit(stunrun_renderer_t *renderer, const uint8_t *source,
                        unsigned source_width, unsigned source_height,
                        int destination_x, int destination_y)
{
    unsigned source_y;
    if (renderer == NULL || source == NULL || source_width == 0u ||
        source_height == 0u)
        return 0;
    for (source_y = 0; source_y < source_height; source_y++) {
        int target_y = destination_y + (int)source_y;
        unsigned source_x;
        if (target_y < 0 || target_y >= (int)STUNRUN_RENDER_HEIGHT)
            continue;
        for (source_x = 0; source_x < source_width; source_x++) {
            int target_x = destination_x + (int)source_x;
            size_t source_offset;
            size_t target_offset;
            if (target_x < 0 || target_x >= (int)STUNRUN_RENDER_WIDTH)
                continue;
            source_offset = ((size_t)source_y * source_width + source_x) * 3u;
            target_offset = ((size_t)target_y * STUNRUN_RENDER_WIDTH +
                             (unsigned)target_x) * 3u;
            renderer->pixels[target_offset] = source[source_offset];
            renderer->pixels[target_offset + 1u] = source[source_offset + 1u];
            renderer->pixels[target_offset + 2u] = source[source_offset + 2u];
        }
    }
    return 1;
}

int stunrun_render_fill_xy(stunrun_renderer_t *renderer, int x0, int y0,
                           int x1, int y1, uint8_t red, uint8_t green,
                           uint8_t blue)
{
    int left;
    int right;
    int top;
    int bottom;
    int y;

    if (renderer == NULL)
        return 0;
    left = x0 < x1 ? x0 : x1;
    right = x0 < x1 ? x1 : x0;
    top = y0 < y1 ? y0 : y1;
    bottom = y0 < y1 ? y1 : y0;
    if (right < 0 || left >= (int)STUNRUN_RENDER_WIDTH ||
        bottom < 0 || top >= (int)STUNRUN_RENDER_HEIGHT)
        return 1;
    if (left < 0)
        left = 0;
    if (top < 0)
        top = 0;
    if (right >= (int)STUNRUN_RENDER_WIDTH)
        right = (int)STUNRUN_RENDER_WIDTH - 1;
    if (bottom >= (int)STUNRUN_RENDER_HEIGHT)
        bottom = (int)STUNRUN_RENDER_HEIGHT - 1;
    for (y = top; y <= bottom; y++) {
        int x;
        for (x = left; x <= right; x++) {
            size_t offset = ((size_t)y * STUNRUN_RENDER_WIDTH +
                             (unsigned)x) * 3u;
            renderer->pixels[offset] = red;
            renderer->pixels[offset + 1u] = green;
            renderer->pixels[offset + 2u] = blue;
        }
    }
    return 1;
}

int stunrun_render_line(stunrun_renderer_t *renderer, int x0, int y0,
                        int x1, int y1, uint8_t red, uint8_t green,
                        uint8_t blue)
{
    int dx;
    int sx;
    int dy;
    int sy;
    int error;

    if (renderer == NULL)
        return 0;
    dx = x0 < x1 ? x1 - x0 : x0 - x1;
    sx = x0 < x1 ? 1 : -1;
    dy = y0 < y1 ? y1 - y0 : y0 - y1;
    sy = y0 < y1 ? 1 : -1;
    error = dx - dy;
    for (;;) {
        if (x0 >= 0 && x0 < (int)STUNRUN_RENDER_WIDTH && y0 >= 0 &&
            y0 < (int)STUNRUN_RENDER_HEIGHT) {
            size_t offset = ((size_t)y0 * STUNRUN_RENDER_WIDTH +
                             (unsigned)x0) * 3u;
            renderer->pixels[offset] = red;
            renderer->pixels[offset + 1u] = green;
            renderer->pixels[offset + 2u] = blue;
        }
        if (x0 == x1 && y0 == y1)
            break;
        {
            int twice_error = error * 2;
            if (twice_error > -dy) {
                error -= dy;
                x0 += sx;
            }
            if (twice_error < dx) {
                error += dx;
                y0 += sy;
            }
        }
    }
    return 1;
}

int stunrun_render_gsp_visible(stunrun_renderer_t *renderer,
                               const uint16_t *vram_words,
                               size_t vram_word_count,
                               const uint8_t *palette_rgb)
{
    unsigned y;
    size_t mask;
    if (renderer == NULL || vram_words == NULL || palette_rgb == NULL ||
        vram_word_count == 0u || (vram_word_count & (vram_word_count - 1u)) != 0u)
        return 0;
    mask = vram_word_count - 1u;
    for (y = 0; y < STUNRUN_RENDER_HEIGHT; y++) {
        size_t row_base = ((size_t)(y / 4u) << 10) & mask;
        size_t color_base = (size_t)(y & 3u) << 9;
        unsigned x;
        for (x = 0; x < STUNRUN_RENDER_WIDTH; x++) {
            size_t color_index = color_base + x;
            uint16_t word = vram_words[(row_base + (color_index >> 1)) & mask];
            unsigned color = (color_index & 1u) ? (word >> 8) : (word & 0xffu);
            size_t source = (size_t)color * 3u;
            size_t target = ((size_t)y * STUNRUN_RENDER_WIDTH + x) * 3u;
            renderer->pixels[target] = palette_rgb[source];
            renderer->pixels[target + 1u] = palette_rgb[source + 1u];
            renderer->pixels[target + 2u] = palette_rgb[source + 2u];
        }
    }
    return 1;
}

uint32_t stunrun_render_hash(const stunrun_renderer_t *renderer)
{
    uint32_t hash = 2166136261u;
    size_t i;
    if (renderer == NULL)
        return 0u;
    for (i = 0; i < STUNRUN_RENDER_BYTES; i++) {
        hash ^= renderer->pixels[i];
        hash *= 16777619u;
    }
    return hash;
}

int stunrun_render_write_ppm(const stunrun_renderer_t *renderer,
                             const char *path)
{
    FILE *file;
    if (renderer == NULL || path == NULL)
        return 0;
    file = fopen(path, "wb");
    if (file == NULL)
        return 0;
    if (fprintf(file, "P6\n%u %u\n255\n", STUNRUN_RENDER_WIDTH,
                STUNRUN_RENDER_HEIGHT) < 0 ||
        fwrite(renderer->pixels, 1, STUNRUN_RENDER_BYTES, file) !=
            STUNRUN_RENDER_BYTES || fclose(file) != 0)
        return 0;
    return 1;
}
