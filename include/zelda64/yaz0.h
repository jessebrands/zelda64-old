#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define ZELDA64_YAZ0_MAGIC "Yaz0"

typedef struct zelda64_yaz0_header {
    char magic[4];
    uint32_t uncompressed_size;
    uint32_t alignment;
} zelda64_yaz0_header_t;

typedef struct zelda64_yaz0_data_group {
    uint8_t header;
    uint8_t chunks[24];
    size_t length;
} zelda64_yaz0_data_group_t;

/**
 * Attempts to read a Yaz0 header from a buffer.
 * @param buf The buffer to read from.
 * @param size The size of the buffer in bytes.
 * @return Structure holding the read data.
 */
zelda64_yaz0_header_t zelda64_get_yaz0_header(const uint8_t *buf, size_t size);

/**
 * Checks whether a Yaz0 header is valid.
 * @param header The header to validate.
 * @return True if the header is valid, false if the header is invalid.
 */
static inline bool zelda64_is_valid_yaz0_header(zelda64_yaz0_header_t header) {
    return memcmp(header.magic, ZELDA64_YAZ0_MAGIC, sizeof(header.magic)) == 0;
}

/**
 * Decompresses Yaz0 data.
 * @param dest Output buffer to write the decompressed data to.
 * @param dest_size Output buffer size in bytes.
 * @param src The source buffer to read the compressed data from.
 */
void zelda64_yaz0_decompress(uint8_t *dest, size_t dest_size, const uint8_t *src);

/**
 * Compresses a portion of the buffer into a Yaz0 data group.
 * @param src The source buffer to read from.
 * @param src_size The size of the source buffer.
 * @param src_pos The position of the buffer to start reading at.
 * @param level The level of compression to use.
 * @param out The output to write to.
 * @return The amount of bytes read from the buffer.
 */
int zelda64_yaz0_compress_group(const uint8_t *src, size_t src_size, int src_pos, int level,
                                zelda64_yaz0_data_group_t *out);
