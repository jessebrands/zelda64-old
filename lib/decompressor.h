#pragma once

#include <stdint.h>

#include "dma.h"

typedef uint8_t *(zelda64_alloc_callback)(size_t size, void *userdata);

typedef void (zelda64_dealloc_callback)(uint8_t *ptr, size_t size, void *userdata);

typedef uint8_t *(zelda64_load_rom_data_callback)(uint32_t offset, uint32_t size, void *userdata);

typedef void (zelda64_unload_rom_data_callback)(uint8_t *data, void *userdata);

typedef void (zelda64_write_callback)(uint64_t offset, size_t size, uint8_t *data, void *userdata);

typedef struct zelda64_decompressor_info {
    zelda64_dma_info_t dma_info;
    const uint8_t *dma_data;
    zelda64_alloc_callback* allocate;
    zelda64_dealloc_callback* deallocate;
    zelda64_load_rom_data_callback* load_rom_data;
    zelda64_unload_rom_data_callback* unload_rom_data;
    zelda64_write_callback* write_out;
    void *userdata;
} zelda64_decompressor_info_t;

int zelda64_decompress_rom(zelda64_decompressor_info_t info);
