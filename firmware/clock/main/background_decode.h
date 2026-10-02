#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define BACKGROUND_WIDTH 480
#define BACKGROUND_HEIGHT 320
#define BACKGROUND_PIXELS (BACKGROUND_WIDTH * BACKGROUND_HEIGHT)
#define BACKGROUND_JPEG_LIMIT (2u * 1024u * 1024u)
/* Worker-only. Caller owns destination and scratch (minimum 8192 bytes).
 * Destination is unpublished until success. No LVGL object/memory APIs used.
 * Optional progress callback cancels stale requests and yields to higher priorities.
 */
bool background_decode_jpeg(const uint8_t *data,size_t size,uint16_t *destination,
                            void *scratch,size_t scratch_size,
                            bool (*proceed)(void *),void *context);

/* position: 0 fill/crop, 1 fit with dark bars, 2 stretch. */
bool background_decode_jpeg_position(const uint8_t *data,size_t size,uint16_t *destination,
    void *scratch,size_t scratch_size,bool (*proceed)(void *),void *context,unsigned position);
