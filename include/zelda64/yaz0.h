#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define ZELDA64_YAZ0_MAGIC "Yaz0"

typedef struct zelda64_yaz0_header {
    char magic[4];
    uint32_t uncompressed_size;
    uint32_t alignment;
} zelda64_yaz0_header_t;

/**
 * Attempts to read a Yaz0 header from a buffer.
 * @param buf The buffer to read from.
 * @param size The size of the buffer in bytes.
 * @return Structure holding the read data.
 */
zelda64_yaz0_header_t zelda64_get_yaz0_header(const uint8_t *buf, size_t size);

/**
 * Decompresses a Yaz0 data stream.
 * @param dest Output buffer to write the decompressed data to.
 * @param dest_size Output buffer size in bytes.
 * @param src The source buffer to read the compressed data from.
 */
void zelda64_yaz0_decompress(uint8_t *dest, uint64_t dest_size, const uint8_t *src);

/**
 * Checks whether a Yaz0 header is valid.
 * @param header The header to validate.
 * @return True if the header is valid, false if the header is invalid.
 */
static inline bool zelda64_is_valid_yaz0_header(zelda64_yaz0_header_t header) {
    return memcmp(header.magic, ZELDA64_YAZ0_MAGIC, sizeof(header.magic)) == 0;
}