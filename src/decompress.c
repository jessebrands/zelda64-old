#include <zelda64/dma.h>
#include <zelda64/yaz0.h>

#include "decompress.h"

typedef enum decompressor_action {
    DECOMPRESSOR_ACTION_SKIP = 0,
    DECOMPRESSOR_ACTION_COPY = 1,
    DECOMPRESSOR_ACTION_DECOMPRESS = 2,
} decompressor_action_t;

static inline decompressor_action_t get_decompressor_action(zelda64_dma_entry_t entry) {
    if (zelda64_is_empty_file(entry)) {
        return DECOMPRESSOR_ACTION_SKIP;
    } else if (zelda64_is_uncompressed_file(entry)) {
        return DECOMPRESSOR_ACTION_COPY;
    } else {
        return DECOMPRESSOR_ACTION_DECOMPRESS;
    }
}

static inline size_t get_file_size(zelda64_dma_entry_t entry, decompressor_action_t action) {
    switch (action) {
        case DECOMPRESSOR_ACTION_COPY:
            return entry.v_end - entry.v_start;
        case DECOMPRESSOR_ACTION_DECOMPRESS:
            return entry.p_end - entry.p_start;
        default:
            return 0;
    }
}

int find_dma_offset(zelda64_decompress_rom_params_t params, size_t *dma_offset) {
    size_t bytes_remaining = params.rom_size;
    size_t bytes_read = 0;
    uint64_t result = 0;
    while (bytes_remaining > 0) {
        // Read the next block of data from the ROM file.
        size_t block_size = params.block_size;
        if (block_size > bytes_remaining) {
            block_size = bytes_remaining;
        }
        uint8_t *data = params.read_rom_data(params.block_size, bytes_read, params.userdata);
        if (data == nullptr) {
            return ZELDA64_DECOMPRESS_INVALID_DATA;
        }
        if (!zelda64_find_dma_table_offset(data, block_size, &result)) {
            *dma_offset = bytes_read + result;
            if (params.close_rom_data != nullptr) {
                params.close_rom_data(data, block_size, params.userdata);
            }
            return ZELDA64_DECOMPRESS_OK;
        }
        bytes_remaining -= block_size;
        bytes_read += block_size;
        if (params.close_rom_data != nullptr) {
            params.close_rom_data(data, block_size, params.userdata);
        }
    }
    return ZELDA64_DECOMPRESS_NO_DMA_TABLE;
}

int get_dma_info(zelda64_decompress_rom_params_t params, zelda64_dma_info_t *out) {
    // Let us gather some information about the DMA table first.
    size_t dma_offset = 0;
    if (find_dma_offset(params, &dma_offset) != ZELDA64_DECOMPRESS_OK) {
        return ZELDA64_DECOMPRESS_NO_DMA_TABLE;
    }
    // Read a small amount of data from the ROM file.
    uint8_t *data = params.read_rom_data(64, dma_offset, params.userdata);
    *out = zelda64_get_dma_table_information(data, 64, 0);
    if (params.close_rom_data != nullptr) {
        params.close_rom_data(data, 64, params.userdata);
    }
    return ZELDA64_DECOMPRESS_OK;
}

zelda64_decompress_result_t zelda64_decompress_rom(zelda64_decompress_rom_params_t params,
                                                   zelda64_allocator_t allocator) {
    zelda64_dma_info_t dma_info = {};
    if (get_dma_info(params, &dma_info)) {
        return ZELDA64_DECOMPRESS_NO_DMA_TABLE;
    }
    // Now we can read in the entire DMA table with relative ease.
    uint8_t *dma_table = params.read_rom_data(dma_info.size, dma_info.offset, params.userdata);
    uint8_t *dma_out = allocator.alloc(dma_info.size, sizeof (uint8_t), allocator.userdata);
    // Pre-allocate the destination.
    params.reserve(params.rom_size * 2, params.userdata);
    for (int_fast32_t i = 0; i < dma_info.entries; ++i) {
        zelda64_dma_entry_t entry = zelda64_get_dma_table_entry(dma_table, dma_info.size, i);
        decompressor_action_t action = get_decompressor_action(entry);
        switch (action) {
            case DECOMPRESSOR_ACTION_SKIP:
                continue;
            case DECOMPRESSOR_ACTION_COPY: {
                size_t size = get_file_size(entry, action);
                if (size > 0) {
                    uint8_t *data = params.read_rom_data(size, entry.p_start, params.userdata);
                    params.write_data(data, size, entry.v_start, params.userdata);
                    if (params.close_rom_data != nullptr) {
                        params.close_rom_data(data, size, params.userdata);
                    }
                }
                break;
            }
            case DECOMPRESSOR_ACTION_DECOMPRESS: {
                size_t size = get_file_size(entry, action);
                uint8_t *data = params.read_rom_data(size, entry.p_start, params.userdata);
                zelda64_yaz0_header_t header = zelda64_get_yaz0_header(data, size);
                if (!zelda64_is_valid_yaz0_header(header)) {
                    if (params.close_rom_data != nullptr) {
                        params.close_rom_data(data, size, params.userdata);
                        params.close_rom_data(dma_table, dma_info.size, params.userdata);
                    }
                    return ZELDA64_DECOMPRESS_INVALID_DATA;
                }
                uint8_t *out_data = allocator.alloc(header.uncompressed_size, sizeof(uint8_t), allocator.userdata);
                zelda64_yaz0_decompress(out_data, header.uncompressed_size, data);
                params.write_data(out_data, header.uncompressed_size, entry.v_start, params.userdata);
                allocator.free(data, allocator.userdata);
                break;
            }
        }
        entry.p_start = entry.v_start;
        entry.p_end = 0;
        zelda64_set_dma_table_entry(dma_out, dma_info.size, i, entry);
    }
    if (params.close_rom_data != nullptr) {
        params.close_rom_data(dma_table, dma_info.size, params.userdata);
    }
    params.write_data(dma_out, dma_info.size, dma_info.offset, params.userdata);
    allocator.free(dma_out, allocator.userdata);
    return ZELDA64_DECOMPRESS_OK;
}
