#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "util.h"

#define YAZ0_MAGIC "Yaz0"

typedef struct zelda64_yaz0_header {
    char magic[4];
    uint32_t uncompressed_size;
    uint32_t alignment;
} zelda64_yaz0_header_t;

static inline zelda64_yaz0_header_t zelda64_get_yaz0_header(uint8_t const *buf) {
    zelda64_yaz0_header_t header = {
            .magic = {0},
            .uncompressed_size = u32_from_buf(buf + 4),
            .alignment =  u32_from_buf(buf + 8),
    };
    memcpy(header.magic, buf, sizeof(header.magic));
    return header;
}

static inline bool zelda64_is_valid_yaz0_header(zelda64_yaz0_header_t header) {
    return memcmp(header.magic, YAZ0_MAGIC, sizeof(header.magic)) == 0;
}

void zelda64_yaz0_decompress(uint8_t *dest, uint64_t dest_size, uint8_t const *src);