//
// Created by Jesse on 29/05/2023.
//

#include <assert.h>
#include <string.h>

#include "rom.h"
#include "util.h"
#include "crc32.h"

#define CRC32_6101_BOOTCODE 0x6170A4A1
#define CRC32_6102_BOOTCODE 0x90BB6CB5
#define CRC32_6103_BOOTCODE 0x0B050EE0
#define CRC32_6105_BOOTCODE 0x98BC2C86
#define CRC32_6106_BOOTCODE 0xACC8580A

static inline uint32_t get_checksum_seed(uint32_t cic) {
    switch (cic) {
        case 6101:
        case 6102:
            return 0xF8CA4DDC;
        case 6103:
            return 0xA3886759;
        case 6105:
            return 0xDF26F436;
        case 6106:
            return 0x1FEA617A;
        default:
            return 0;
    }
}

void zelda64_read_rom_header_from_buffer(zelda64_rom_header_t *destination, uint8_t const *const buffer,
                                         size_t buffer_length) {
    assert(destination != NULL);
    assert(buffer != NULL);
    if (buffer_length >= 64) {
        destination->pi_settings = u32_from_buf(buffer);
        destination->clock_rate = u32_from_buf(buffer + 4);
        destination->program_counter = u32_from_buf(buffer + 8);
        destination->release_address = u32_from_buf(buffer + 12);
        destination->crc1_checksum = u32_from_buf(buffer + 16);
        destination->crc2_checksum = u32_from_buf(buffer + 20);
        memcpy(destination->image_name, buffer + 32, ZELDA64_ROM_IMAGE_NAME_LENGTH);
        destination->format = *((char *) buffer + 59);
        memcpy(destination->game_id, buffer + 60, ZELDA64_GAME_ID_LENGTH);
        destination->region_id = *((char *) buffer + 62);
        destination->version = *(buffer + 63);
    }
    if (buffer_length >= 4096) {
        memcpy(destination->bootcode, buffer + 64, ZELDA64_BOOTCODE_LENGTH);
    }
}

uint32_t zelda64_calculate_rom_cic(uint8_t const *bootcode, uint64_t size) {
    uint32_t crc = zelda64_crc32_calculate_checksum(bootcode, size);
    switch (crc) {
        case CRC32_6101_BOOTCODE:
            return 6101;
        case CRC32_6102_BOOTCODE:
            return 6012;
        case CRC32_6103_BOOTCODE:
            return 6103;
        case CRC32_6105_BOOTCODE:
            return 6105;
        case CRC32_6106_BOOTCODE:
            return 6106;
        default:
            return 0;
    }
}

void zelda64_calculate_rom_checksum(uint8_t const *restrict data, uint64_t size, uint32_t cic,
                                    uint32_t *restrict crc1, uint32_t *restrict crc2) {
    assert(size >= 0x101000);
    uint32_t seed = get_checksum_seed(cic);
    uint32_t t1, t2, t3, t4, t5, t6;
    t1 = seed;
    t2 = seed;
    t3 = seed;
    t4 = seed;
    t5 = seed;
    t6 = seed;
    for (int_fast32_t i = 0x1000; i < 0x101000; i += 4) {
        uint32_t d = u32_from_buf(data + i);
        if ((t6 + d) < t6) t4++;
        t6 += d;
        t3 ^= d;
        uint32_t r = (d << (d & 0x1F)) | (d >> (32 - (d & 0x1F)));
        t5 += r;
        if (t2 > d) t2 ^= r;
        else t2 ^= t6 ^ d;
        if (cic == 6105) {
            uint32_t p = 0x0750 + (i & 0xFF);
            uint32_t b = u32_from_buf(data + p);
            t1 += b ^ d;
        } else {
            t1 += t5 ^ d;
        }
        // calculate the CRC
        if (cic == 6103) {
            *crc1 = (t6 ^ t4) + t3;
            *crc2 = (t5 ^ t2) + t1;
        } else if (cic == 6106) {
            *crc1 = (t6 * t4) + t3;
            *crc2 = (t5 * t2) + t1;
        } else {
            *crc1 = t6 ^ t4 ^ t3;
            *crc2 = t5 ^ t2 ^ t1;
        }
    }
}
