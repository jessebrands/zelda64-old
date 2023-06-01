#include <zelda64/crc32.h>

static uint32_t crc_table[256] = {0};

#define CRC32_POLY 0xEDB88320

static inline void generate_crc32_table() {
    for (int_fast32_t i = 0; i < sizeof crc_table; ++i) {
        uint32_t crc = i;
        for (int_fast32_t j = 8; j > 0; --j) {
            if (crc & 1) {
                crc = (crc >> 1) ^ CRC32_POLY;
            } else {
                crc >>= 1;
            }
        }
        crc_table[i] = crc;
    }
}

uint32_t zelda64_crc32_calculate_checksum(const uint8_t *data, size_t size) {
    if (crc_table[0] == 0) {
        generate_crc32_table();
    }
    uint32_t crc = 0xFFFFFFFF;
    for (int_fast32_t i = 0; i < size; ++i) {
        uint8_t const ch = data[i];
        uint8_t const t = (crc ^ ch) & 0xFF;
        crc = (crc >> 8) ^ crc_table[t];
    }
    return ~crc;
}
