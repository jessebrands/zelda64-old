#include <zelda64/yaz0.h>
#include <assert.h>

#include "util.h"

zelda64_yaz0_header_t zelda64_get_yaz0_header(const uint8_t *buf, size_t size) {
    assert(size >= 16);
    zelda64_yaz0_header_t header = {
            .magic = {0},
            .uncompressed_size = u32_from_buf(buf + 4),
            .alignment = u32_from_buf(buf + 8),
    };
    memcpy(header.magic, buf, sizeof(header.magic));
    return header;
}

void zelda64_yaz0_decompress(uint8_t *dest, uint64_t dest_size, const uint8_t *src) {
    uint64_t src_index = 16; // skip header
    uint64_t dest_index = 0;
    uint8_t count = 0;
    uint8_t cb;
    while (dest_index < dest_size) {
        if (count == 0) {
            cb = src[src_index++];
            count = 8;
        }
        // Check the 7th bit, if it's a 1 we can just copy a byte. :-)
        if (cb & 0x80) {
            dest[dest_index++] = src[src_index++];
        } else {
            uint8_t b1 = src[src_index++];
            uint8_t b2 = src[src_index++];
            uint32_t distance = (((b1 & 0xF) << 8) | b2) + 1;
            uint32_t cursor = dest_index - distance;
            uint32_t bytes = b1 >> 4;
            if (bytes == 0) {
                bytes = src[src_index++] + 0x12;
            } else {
                bytes += 2;
            }
            for (uint32_t n = 0; n < bytes; ++n) {
                dest[dest_index++] = dest[cursor++];
            }
        }
        cb = cb << 1;
        count--;
    }
}
