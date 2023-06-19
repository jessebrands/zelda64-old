#include <assert.h>
#include <stdio.h>
#include <pthread.h>

#include <zelda64/dma.h>
#include <zelda64/yaz0.h>

#include "compress.h"
#include "../lib/util.h"

typedef enum compressor_action {
    COMPRESSOR_ACTION_SKIP = 0,
    COMPRESSOR_ACTION_COPY = 1,
    COMPRESSOR_ACTION_COMPRESS = 2,
} compressor_action_t;

typedef struct compressor_worker_params {
    const zelda64_compress_rom_params_t *compress_params;
    void *data;
    size_t size;
    size_t write_offset;
} compressor_worker_params_t;

#define COMPRESSION_BUFFER_SIZE 4096

size_t compress_worker(void *userdata) {
    assert(userdata != nullptr);
    compressor_worker_params_t *params = (compressor_worker_params_t *) userdata;
    uint8_t buffer[COMPRESSION_BUFFER_SIZE] = {};
    // First, write the Yaz0 header.
    zelda64_yaz0_header_t header = {
            .magic = ZELDA64_YAZ0_MAGIC,
            .uncompressed_size = params->size,
    };
    memcpy(buffer, header.magic, 4);
    u32_to_buf(header.uncompressed_size, buffer + 4);
    size_t bytes_written = 16;
    // Now we can move forward with decompression!
    int bytes_read = 0;
    size_t bytes_out = 0;
    while (bytes_read < params->size) {
        if (bytes_written + 25 > sizeof buffer) {
            // Write out the buffer we got so far and reset it.
            params->compress_params->write_data(buffer, bytes_written, params->write_offset + bytes_out,
                                                params->compress_params->userdata);
            bytes_out += bytes_written;
            memset(buffer, 0, sizeof buffer);
            bytes_written = 0;
        }
        zelda64_yaz0_data_group_t group = {};
        bytes_read = zelda64_yaz0_compress_group(params->data, params->size, bytes_read, 9, &group);
        // Copy the group to the intermediate compression buffer.
        buffer[bytes_written] = group.header;
        memcpy(buffer + bytes_written + 1, group.chunks, group.length);
        bytes_written += group.length + 1;
    }
    // Finalize writing data.
    params->compress_params->write_data(buffer, bytes_written, params->write_offset + bytes_out,
                                        params->compress_params->userdata);
    // File sizes must be aligned so do that here:
    bytes_written = (bytes_out + bytes_written + 31) & -16;
    return bytes_written;
}

zelda64_result_t zelda64_compress_rom(zelda64_compress_rom_params_t params, zelda64_allocator_t allocator) {
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
    // Read the entire DMA table into memory, too.
    uint8_t *dma_table = params.read_rom_data(dma_info.size, dma_info.offset, params.userdata);
    uint8_t *dma_out = allocator.alloc(dma_info.size, sizeof(uint8_t), allocator.userdata);
    compressor_action_t *actions = allocator.alloc(dma_info.entries, sizeof(compressor_action_t), allocator.userdata);
    for (int_fast32_t i = 0; i < dma_info.entries; ++i) {
        actions[i] = COMPRESSOR_ACTION_COMPRESS;
    }
    // Any excluded files should simply be copied over from the source.
    for (int_fast32_t i = 0; i < params.exclusion_list_size; ++i) {
        uint32_t exclusion = params.exclusion_list[i];
        actions[exclusion] = COMPRESSOR_ACTION_COPY;
    }
    // The way to do this is a bit odd. The plan is to essentially read from the source rom sequentially. A DMA table
    // should, technically speaking, be in the correct order.
    size_t cursor = 0;
    for (int_fast32_t i = 0; i < dma_info.entries; ++i) {
        // Read the data from the ROM.
        zelda64_dma_entry_t entry = zelda64_get_dma_table_entry(dma_table, dma_info.size, i);
        if (entry.v_start != entry.v_end) {
            size_t uncompressed_size = zelda64_get_file_size(entry);
            uint8_t *data = params.read_rom_data(uncompressed_size, entry.p_start, params.userdata);
            entry.p_start = cursor;
            if (actions[i] == COMPRESSOR_ACTION_COMPRESS) {
                printf("compressing file %d/%d\n", i + 1, dma_info.entries);
                compressor_worker_params_t worker_params = {
                        .data = data,
                        .size = uncompressed_size,
                        .compress_params = &params,
                        .write_offset = cursor,
                };
                size_t compressed_size = compress_worker(&worker_params);
                entry.p_end = cursor + compressed_size;
                cursor += compressed_size; // advance write cursor
            } else if (actions[i] == COMPRESSOR_ACTION_COPY) {
                printf("copying file %d/%d\n", i + 1, dma_info.entries);
                params.write_data(data, uncompressed_size, cursor, params.userdata);
                cursor += uncompressed_size;
            } else {
                printf("skipping file %d/%d\n", i + 1, dma_info.entries);
                entry.p_start = 0xFF'FF'FF'FF;
                entry.p_end = 0xFF'FF'FF'FF;
            }
            params.close_rom_data(data, uncompressed_size, params.userdata);
        } else {
            printf("skipping dead file at %d/%d\n", i + 1, dma_info.entries);
        }
        // Write the entry into the DMA table.
        zelda64_set_dma_table_entry(dma_out, dma_info.size, i, entry);
    }
    params.close_rom_data(dma_table, dma_info.size, params.userdata);
    params.write_data(dma_out, dma_info.size, dma_info.offset, params.userdata);
    allocator.free(dma_out, allocator.userdata);
    return ZELDA64_OK;
}
