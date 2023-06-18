#pragma once

#include <stddef.h>
#include <stdint.h>

#include <zelda64/dma.h>
#include <zelda64/zelda64.h>

typedef struct zelda64_decompress_rom_params {
    // Callback function that requests data from the ROM file. This is a required function and must return an uint8_t
    // array of at least `size` bytes long.
    zelda64_read_data_func_t *read_rom_data;

    // Optional callback function that is called after a piece of data is no longer needed.
    zelda64_close_data_func_t *close_rom_data;

    // Callback to reserve space or allocate an output buffer.
    void (*reserve)(size_t size, void *userdata);

    // Write callback function.
    zelda64_write_data_func_t *write_data;

    // When reading sequentially over the ROM, this is the largest block of data the compressor will request at a
    // time. It is recommended to set this to something large that is a multiple of 16, like 8K or 16K.
    size_t block_size;

    // The size of the ROM in bytes.
    size_t rom_size;

    // Pointer to user data that will be passed in to any callback functions.
    void *userdata;
} zelda64_decompress_rom_params_t;

zelda64_result_t zelda64_decompress_rom(zelda64_decompress_rom_params_t params,
                                        zelda64_allocator_t allocator);
