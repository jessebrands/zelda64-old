#pragma once

#include <stddef.h>
#include <stdint.h>

#include <zelda64/dma.h>

#include "zelda64.h"

typedef enum zelda64_decompress_result {
    ZELDA64_DECOMPRESS_OK = 0,
    ZELDA64_DECOMPRESS_INVALID_DATA = 1,
    ZELDA64_DECOMPRESS_NO_DMA_TABLE = 2,
} zelda64_decompress_result_t;

typedef struct zelda64_decompress_rom_params {
    // Callback function that requests data from the ROM file. This is a required function and must return an uint8_t
    // array of at least `size` bytes long.
    uint8_t* (*read_rom_data)(size_t size, size_t offset, void *userdata);

    // Optional callback function that is called after a piece of data is no longer needed.
    void (*close_rom_data)(uint8_t *data, size_t size, void *userdata);

    // Callback to reserve space or allocate an output buffer.
    void (*reserve)(size_t size, void *userdata);

    // Write callback function.
    void (*write_data)(uint8_t *data, size_t size, size_t offset, void *userdata);

    // When reading sequentially over the ROM, this is the largest block of data the compressor will request at a
    // time. It is recommended to set this to something large that is a multiple of 16, like 8K or 16K.
    size_t block_size;

    // The size of the ROM in bytes.
    size_t rom_size;

    // Pointer to user data that will be passed in to any callback functions.
    void *userdata;
} zelda64_decompress_rom_params_t;

zelda64_decompress_result_t zelda64_decompress_rom(zelda64_decompress_rom_params_t params,
                                                   zelda64_allocator_t allocator);
