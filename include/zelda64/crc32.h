#pragma once

#include <stdint.h>

/**
 * Calculates a CRC-32 checksum for a given piece of data.
 * @param data The buffer holding the data.
 * @param size The size of the buffer in bytes.
 * @return CRC-32 checksum represented as a 32-bit unsigned integer.
 */
uint32_t zelda64_crc32_calculate_checksum(const uint8_t *data, size_t size);
