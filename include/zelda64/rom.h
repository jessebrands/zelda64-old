#pragma once

#include <stddef.h>
#include <stdint.h>

#define ZELDA64_ROM_IMAGE_NAME_LENGTH 20
#define ZELDA64_GAME_ID_LENGTH 2
#define ZELDA64_BOOTCODE_LENGTH 4032

typedef struct zelda64_rom_header {
    uint32_t pi_settings;
    uint32_t clock_rate;
    uint32_t program_counter;
    uint32_t release_address;
    uint32_t crc1_checksum;
    uint32_t crc2_checksum;
    char image_name[ZELDA64_ROM_IMAGE_NAME_LENGTH];
    char format;
    char game_id[ZELDA64_GAME_ID_LENGTH];
    char region_id;
    uint8_t version;
    uint8_t bootcode[ZELDA64_BOOTCODE_LENGTH];
} zelda64_rom_header_t;

void zelda64_read_rom_header_from_buffer(zelda64_rom_header_t *destination, const uint8_t *buffer,
                                         size_t buffer_length);

/**
 * Derives the CIC chip used by the ROM.
 * @param bootcode The bootcode of the ROM.
 * @param size The size of the buffer holding the bootcode in bytes.
 * @return Integer specifying the CIC chip.
 */
uint32_t zelda64_calculate_rom_cic(const uint8_t *bootcode, size_t size);

/**
 * Calculates the N64 ROM checksum.
 * @param data Buffer holding data to calculate checksum for.
 * @param size The size of the buffer.
 * @param cic The CIC used by this ROM.
 * @param crc1 Output parameter holding the first CRC-32 value.
 * @param crc2 Output parameter holding the second CRC-32 value.
 */
void zelda64_calculate_rom_checksum(const uint8_t *data, size_t size, uint32_t cic, uint32_t *crc1, uint32_t *crc2);
