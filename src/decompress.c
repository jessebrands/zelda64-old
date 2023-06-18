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

zelda64_result_t zelda64_decompress_rom(zelda64_decompress_rom_params_t params,
                                                   zelda64_allocator_t allocator) {
    zelda64_dma_info_t dma_info = {};
    zelda64_find_dma_table_params_t find_dma_table_params = {
            .rom_size = params.rom_size,
            .block_size = params.block_size,
            .read_block = params.read_rom_data,
            .close_block = params.close_rom_data,
            .userdata = params.userdata,
    };
    if (zelda64_find_dma_table(find_dma_table_params, &dma_info) != ZELDA64_OK) {
        return ZELDA64_ERROR_INVALID_DATA;
    }
    // Now we can read in the entire DMA table with relative ease.
    uint8_t *dma_table = params.read_rom_data(dma_info.size, dma_info.offset, params.userdata);
    uint8_t *dma_out = allocator.alloc(dma_info.size, sizeof (uint8_t), allocator.userdata);
    // Pre-allocate the destination.
    params.reserve(params.rom_size * 2, params.userdata);
    for (int_fast32_t i = 0; i < dma_info.entries; ++i) {
        zelda64_dma_entry_t entry = zelda64_get_dma_table_entry(dma_table, dma_info.size, i);
        decompressor_action_t action = get_decompressor_action(entry);
        size_t size = zelda64_get_file_size(entry);
        switch (action) {
            case DECOMPRESSOR_ACTION_SKIP:
                continue;
            case DECOMPRESSOR_ACTION_COPY: {
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
                uint8_t *data = params.read_rom_data(size, entry.p_start, params.userdata);
                zelda64_yaz0_header_t header = zelda64_get_yaz0_header(data, size);
                if (!zelda64_is_valid_yaz0_header(header)) {
                    if (params.close_rom_data != nullptr) {
                        params.close_rom_data(data, size, params.userdata);
                        params.close_rom_data(dma_table, dma_info.size, params.userdata);
                    }
                    return ZELDA64_ERROR_INVALID_DATA;
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
    return ZELDA64_OK;
}
