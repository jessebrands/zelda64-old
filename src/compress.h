#pragma once

#include <zelda64/zelda64.h>

typedef struct zelda64_compress_rom_params {
    zelda64_read_data_func_t *read_rom_data;
    zelda64_close_data_func_t *close_rom_data;
    zelda64_write_data_func_t *write_data;
    uint32_t *exclusion_list;
    size_t exclusion_list_size;
    size_t rom_size;
    size_t block_size;
    size_t threshold;
    void *userdata;
} zelda64_compress_rom_params_t;

zelda64_result_t zelda64_compress_rom(zelda64_compress_rom_params_t params, zelda64_allocator_t allocator);