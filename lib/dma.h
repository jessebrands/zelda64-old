#pragma once

#include <stdint.h>

typedef struct zelda64_dma_entry {
    uint32_t v_start;
    uint32_t v_end;
    uint32_t p_start;
    uint32_t p_end;
} zelda64_dma_entry_t;

typedef struct zelda64_dma_info {
    uint32_t offset;
    uint32_t size;
    uint32_t entries;
} zelda64_dma_info_t;

/**
 * Seeks the start of a DMA table within a buffer.
 * @param buffer The buffer to search.
 * @param buffer_length The length of the buffer.
 * @param offset Address of an integer to write the offset to. Only changed if this function returns 0.
 * @return Will be non-zero on failure, and zero on success.
 */
int zelda64_find_dma_table_offset(const uint8_t *restrict buffer, uint64_t buffer_length, uint64_t *restrict offset);

/**
 * Returns information about the DMA table.
 * @param buffer The buffer which to read from.
 * @param dma_offset Offset at which the DMA table begins in the buffer.
 * @return Structure containing information about the DMA table.
 * @note The returned structure may contain invalid information if the function was supplied incorrect argument.
 */
zelda64_dma_info_t zelda64_get_dma_table_information(uint8_t const *buffer, uint64_t dma_offset);

zelda64_dma_entry_t zelda64_get_dma_table_entry(uint8_t const *dma_data, uint32_t size, uint32_t index);

int zelda64_set_dma_table_entry(uint8_t *dma_data, uint32_t size, uint32_t index, zelda64_dma_entry_t entry);