//
// Created by Jesse on 30/05/2023.
//

#include "yaz0.h"

void zelda64_yaz0_decompress(uint8_t *restrict dest, uint64_t dest_size,
                             uint8_t const *restrict src, uint64_t src_size) {
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
