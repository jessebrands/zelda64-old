#pragma once

#include <stdint.h>

#include <zelda64/dma.h>

typedef enum zelda64_decompressor_action {
    ZELDA64_ACTION_SKIP = 0,
    ZELDA64_ACTION_COPY = 1,
    ZELDA64_ACTION_DECOMPRESS = 2,
} zelda64_decompressor_action_t;

/**
 * Returns the action that should be taken for a ROM internal file.
 * @param entry The DMA entry of the file to check.
 * @return The action to be taken on this file.
 */
static inline zelda64_decompressor_action_t zelda64_get_decompressor_action(zelda64_dma_entry_t entry) {
    if (zelda64_is_empty_file(entry)) {
        return ZELDA64_ACTION_SKIP;
    } else if(zelda64_is_uncompressed_file(entry)) {
        return ZELDA64_ACTION_COPY;
    } else {
        return ZELDA64_ACTION_DECOMPRESS;
    }
}

/**
 * Returns the size of an internal ROM file.
 * @param entry The DMA table entry representing the file.
 * @param action The decompression action to take on this file.
 * @return 0 if the file does not exist, otherwise a non-0 size in bytes.
 */
static inline size_t zelda64_get_file_size(zelda64_dma_entry_t entry, zelda64_decompressor_action_t action) {
    switch (action) {
        case ZELDA64_ACTION_COPY:
            return entry.v_end - entry.v_start;
        case ZELDA64_ACTION_DECOMPRESS:
            return entry.p_end - entry.p_start;
        default:
            return 0;
    }
}

/// Function pointer type for buffer allocation callbacks.
typedef uint8_t *(zelda64_alloc_callback)(size_t size, void *userdata);

/// Function pointer type for buffer deallocation callbacks.
typedef void (zelda64_dealloc_callback)(uint8_t *ptr, size_t size, void *userdata);

/// Function pointer type for ROM data loading callbacks.
typedef uint8_t *(zelda64_load_rom_data_callback)(uint32_t offset, uint32_t size, void *userdata);

/// Function pointer type for ROM data unloading callbacks.
typedef void (zelda64_unload_rom_data_callback)(uint8_t *data, void *userdata);

/// Function pointer type for ROM data write-out callbacks.
typedef void (zelda64_write_callback)(uint64_t offset, size_t size, uint8_t *data, void *userdata);

/**
 * Structure specifying parameters to the decompression algorithm.
 */
typedef struct zelda64_decompressor_info {
    /// DMA information structure.
    zelda64_dma_info_t dma_info;

    /// Pointer to a buffer holding DMA data. It should be at least as large as dma_info.size.
    const uint8_t *dma_data;

    /// Pointer to a function that allocates a buffer.
    zelda64_alloc_callback *allocate;

    /// Pointer to a function that frees an allocated buffer.
    zelda64_dealloc_callback *deallocate;

    /// Pointer to a function that reads data from the source ROM.
    zelda64_load_rom_data_callback *load_rom_data;

    /// Pointer to a function that unloads the data from the source ROM. May be null.
    zelda64_unload_rom_data_callback *unload_rom_data;

    /// Pointer to a function that writes decompressed data to the output ROM.
    zelda64_write_callback *write_out;

    /// Pointer to user data to supplied to the callbacks.
    void *userdata;
} zelda64_decompressor_info_t;

/**
 * Decompresses a Nintendo 64 Zelda ROM.
 *
 * This function provides a convenient, higher level API over the Zelda 64 API. This function implements a correct,
 * fast algorithm for decompressing a Zelda 64 ROM. You must supply a struct with pointers to functions implementing
 * the necessary I/O and memory management calls.
 *
 * @param info Structure with parameters for the decompressor algorithm.
 * @return 0 on success, non-0 on failure.
 * @see zelda64_decompressor_info_t
 */
int zelda64_decompress_rom(zelda64_decompressor_info_t info);
