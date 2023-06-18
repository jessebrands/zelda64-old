#pragma once

#include <stdbool.h>
#include <stddef.h>
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
 * @param out Address of an integer to write the offset to. Only changed if this function returns 0.
 * @return Will be non-zero on failure, and zero on success.
 */
int zelda64_find_dma_table_offset(const uint8_t *buffer, size_t buffer_length, uint64_t *out);

/**
 * Returns information about the DMA table.
 * @param buffer The buffer which to read from.
 * @param dma_offset Offset at which the DMA table begins in the buffer.
 * @return Structure containing information about the DMA table.
 * @note The returned structure may contain invalid information if the function was supplied incorrect argument.
 */
zelda64_dma_info_t zelda64_get_dma_table_information(const uint8_t *buffer, size_t buffer_size, size_t dma_offset);

/**
 * Retrieves a DMA entry from a DMA table.
 * @param dma_data Buffer containing the DMA table.
 * @param size Size of the DMA table buffer in bytes.
 * @param index Index of the DMA entry in the table.
 * @return The DMA entry.
 */
zelda64_dma_entry_t zelda64_get_dma_table_entry(const uint8_t *dma_data, size_t size, uint32_t index);

/**
 * Writes out a DMA entry into a DMA table.
 * @param dma_data Buffer containing the DMA table.
 * @param size Size of the DMA table buffer in bytes.
 * @param index Index of the DMA entry in the table.
 * @param entry The entry to write to the DMA table.
 * @return 0 if successful, non-0 on failure.
 */
int zelda64_set_dma_table_entry(uint8_t *dma_data, size_t size, uint32_t index, zelda64_dma_entry_t entry);

/**
 * Checks whether a DMA entry points to an empty file.
 * @param entry The DMA entry to check.
 * @return true if the entry points to a nil-file, false if not.
 */
static inline bool zelda64_is_empty_file(zelda64_dma_entry_t entry) {
    return entry.p_start >= 0x4000000 || entry.p_end == 0xFFFFFFFF;
}

/**
 * Checks whether a DMA entry points to an uncompressed file.
 * @param entry The DMA entry to check.
 * @return true if the entry points to an uncompressed file, false if not.
 */
static inline bool zelda64_is_uncompressed_file(zelda64_dma_entry_t entry) {
    return entry.p_end == 0;
}

/**
 * Checks whether a DMA entry points to a compressed file.
 * @param entry The DMA entry to check.
 * @return true if the entry points to a compressed file, false if not.
 */
static inline bool zelda64_is_compressed_file(zelda64_dma_entry_t entry) {
    return !zelda64_is_empty_file(entry) && !zelda64_is_uncompressed_file(entry);
}


