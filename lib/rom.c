//
// Created by Jesse on 29/05/2023.
//

#include <assert.h>
#include <string.h>

#include "rom.h"
#include "util.h"

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
