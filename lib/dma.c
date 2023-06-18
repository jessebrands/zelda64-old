#include <assert.h>
#include <string.h>

#include <zelda64/dma.h>
#include "util.h"

static inline zelda64_dma_entry_t read_entry_from_buf(const uint8_t *buf) {
    return (zelda64_dma_entry_t) {
            u32_from_buf(buf),
            u32_from_buf(buf + 4),
            u32_from_buf(buf + 8),
            u32_from_buf(buf + 12),
    };
}

static inline void write_entry_to_buf(uint8_t *buf, zelda64_dma_entry_t entry) {
    u32_to_buf(entry.v_start, buf);
    u32_to_buf(entry.v_end, buf + 4);
    u32_to_buf(entry.p_start, buf + 8);
    u32_to_buf(entry.p_end, buf + 12);
}

static const uint8_t needle[] = {
        0x00, 0x00, 0x00, 0x00, // v_start1
        0x00, 0x00, 0x10, 0x60, // v_end1
        0x00, 0x00, 0x00, 0x00, // p_start1
        0x00, 0x00, 0x00, 0x00, // p_end1
        0x00, 0x00, 0x10, 0x60, // v_start2
};

int zelda64_find_dma_table_offset(const uint8_t *buffer, size_t buffer_length, uint64_t *out) {
    assert(buffer != NULL);
    assert(out != NULL);
    for (size_t offset = 0; offset + sizeof(needle) < buffer_length; offset += 4) {
        const uint8_t *cursor = buffer + offset;
        if (memcmp(cursor, needle, sizeof(needle)) == 0) {
            *out = offset;
            return 0;
        }
    }
    return -1;
}

zelda64_dma_info_t zelda64_get_dma_table_information(const uint8_t *buffer, size_t buffer_size, size_t dma_offset) {
    assert(buffer != NULL);
    const uint8_t *cursor = buffer + (dma_offset + 32);
    const zelda64_dma_entry_t entry = read_entry_from_buf(cursor);
    return (zelda64_dma_info_t) {
            entry.v_start,
            entry.v_end - entry.v_start,
            (entry.v_end - entry.v_start) / 16
    };
}

zelda64_result_t seek_dma_offset(zelda64_find_dma_table_params_t params, size_t *dma_offset) {
    size_t bytes_remaining = params.rom_size;
    size_t bytes_read = 0;
    uint64_t result = 0;
    while (bytes_remaining > 0) {
        // Read the next block of data from the ROM file.
        size_t block_size = params.block_size;
        if (block_size > bytes_remaining) {
            block_size = bytes_remaining;
        }
        uint8_t *data = params.read_block(params.block_size, bytes_read, params.userdata);
        if (data == nullptr) {
            return ZELDA64_ERROR_INVALID_DATA;
        }
        if (!zelda64_find_dma_table_offset(data, block_size, &result)) {
            *dma_offset = bytes_read + result;
            if (params.close_block != nullptr) {
                params.close_block(data, block_size, params.userdata);
            }
            return ZELDA64_OK;
        }
        bytes_remaining -= block_size;
        bytes_read += block_size;
        if (params.close_block != nullptr) {
            params.close_block(data, block_size, params.userdata);
        }
    }
    return ZELDA64_ERROR_INVALID_DATA;
}

zelda64_result_t zelda64_find_dma_table(zelda64_find_dma_table_params_t params, zelda64_dma_info_t *out) {
    size_t dma_offset = 0;
    if (seek_dma_offset(params, &dma_offset) != ZELDA64_OK) {
        return ZELDA64_ERROR_INVALID_DATA;
    }
    // Read first 3 entries from the DMA table.
    uint8_t *data = params.read_block(48, dma_offset, params.userdata);
    *out = zelda64_get_dma_table_information(data, 48, 0);
    if (params.close_block != nullptr) {
        params.close_block(data, 48, params.userdata);
    }
    return ZELDA64_OK;
}

zelda64_dma_entry_t zelda64_get_dma_table_entry(const uint8_t *dma_data, size_t size, uint32_t index) {
    assert(dma_data != NULL);
    assert(index * 16 + 16 <= size);
    const uint8_t *cursor = dma_data + index * 16;
    return read_entry_from_buf(cursor);
}

int zelda64_set_dma_table_entry(uint8_t *dma_data, size_t size, uint32_t index, zelda64_dma_entry_t entry) {
    assert(dma_data != NULL);
    assert(index * 16 + 16 <= size);
    write_entry_to_buf(dma_data + 16 * index, entry);
    return 0;
}

