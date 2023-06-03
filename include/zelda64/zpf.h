#pragma once

#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define ZPF1_MAGIC "ZPFv1"

/*
 * ZPF Patch File Specification
 * ----------------------------
 *
 * ZPF is a format created to patch Nintendo 64 Zelda ROMs, though in theory the format should be able to work with
 * just about any Nintendo 64 ROM.
 *
 * Every ZPF file starts with a brief header:
 *
 * Offset(b)   Length     Description
 * 0           char[5]    ZPF Magic value "ZPFv1"
 * 5           uint32_t   DMA start offset (in the target ROM)
 * 9           uint32_t   XOR range start offset
 * 13          uint32_t   XOR range end offset
 * 17          uint32_t   XOR address
 *
 * What follows then is a list of DMA entries that must be modified in the target ROM:
 *
 * Offset(b)   Length     Description
 * 0           uint16_t   Index in the DMA table. 0xFFFF marks the end of the table.
 * 2           uint32_t   The original file in the ROM that must be patched.
 * 6           uint32_t   The start offset to write to in the ROM.
 * 10          uint24_t   The size of the data to write in the ROM.
 *
 * After the table follows a bunch of XOR data blocks, which will be used to overwrite the data in the ROM.
 */

typedef struct zelda64_zpf1_header {
    char magic[5];
    uint32_t dma_offset;
    uint32_t xor_start;
    uint32_t xor_end;
    uint32_t xor_address;

} zelda64_zpf1_header_t;

typedef struct zelda64_zpf_dma_entry {
    uint16_t index;
    uint32_t from_file;
    uint32_t offset;
    uint32_t size;
} zelda64_zpf_dma_entry_t;

/**
 * Checks whether the data in the buffer is ZPF1 data.
 * @param data The buffer to read ZPF data from.
 * @param size The size of the buffer in bytes.
 * @return True if the buffer has a ZPF header, false if not.
 */
static inline bool zelda64_is_zpf1_data(const uint8_t *data, size_t size) {
    return size >= sizeof ZPF1_MAGIC && memcmp(data, ZPF1_MAGIC, sizeof ZPF1_MAGIC) == 0;
}

int zelda64_zpf1_read_header(zelda64_zpf1_header_t *out, const uint8_t *data, size_t size);

typedef struct zelda64_zpf_dma_reader {
    int_fast32_t index;
    const uint8_t *data;
    size_t offset;
    size_t size;
} zelda64_zpf_dma_reader_t;

bool zelda64_zpf_get_next_dma_entry(zelda64_zpf_dma_reader_t *reader, zelda64_zpf_dma_entry_t *dma_entry);
