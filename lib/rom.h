#pragma once

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

void zelda64_read_rom_header_from_buffer(zelda64_rom_header_t *destination, uint8_t const *buffer,
                                         size_t buffer_length);
