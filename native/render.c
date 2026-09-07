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
