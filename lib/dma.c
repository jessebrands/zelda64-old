#include <assert.h>
#include <string.h>

#include <zelda64/dma.h>
#include "util.h"

static inline zelda64_dma_entry_t read_entry_from_buf(const uint8_t *buf) {
    return (zelda64_dma_entry_t) {
            u32_from_buf(buf),
            u32_from_buf(buf + 4),
            u32_from_buf(buf + 8),
            u32_from_buf(buf + 12),
    };
}

static inline void write_entry_to_buf(uint8_t *buf, zelda64_dma_entry_t entry) {
    u32_to_buf(entry.v_start, buf);
    u32_to_buf(entry.v_end, buf + 4);
    u32_to_buf(entry.p_start, buf + 8);
    u32_to_buf(entry.p_end, buf + 12);
}

static const uint8_t needle[] = {
        0x00, 0x00, 0x00, 0x00, // v_start1
        0x00, 0x00, 0x10, 0x60, // v_end1
        0x00, 0x00, 0x00, 0x00, // p_start1
        0x00, 0x00, 0x00, 0x00, // p_end1
        0x00, 0x00, 0x10, 0x60, // v_start2
};

int zelda64_find_dma_table_offset(const uint8_t *buffer, size_t buffer_length, uint64_t *out) {
    assert(buffer != NULL);
    assert(out != NULL);
    for (size_t offset = 0; offset + sizeof(needle) < buffer_length; offset += 4) {
        const uint8_t *cursor = buffer + offset;
        if (memcmp(cursor, needle, sizeof(needle)) == 0) {
            *out = offset;
            return 0;
        }
    }
    return -1;
}

zelda64_dma_info_t zelda64_get_dma_table_information(const uint8_t *buffer, size_t dma_offset) {
    assert(buffer != NULL);
    const uint8_t *cursor = buffer + (dma_offset + 32);
    const zelda64_dma_entry_t entry = read_entry_from_buf(cursor);
    return (zelda64_dma_info_t) {
            entry.v_start,
            entry.v_end - entry.v_start,
            (entry.v_end - entry.v_start) / 16
    };
}

zelda64_dma_entry_t zelda64_get_dma_table_entry(const uint8_t *dma_data, size_t size, uint32_t index) {
    assert(dma_data != NULL);
    assert(index * 16 + 16 <= size);
    const uint8_t *cursor = dma_data + index * 16;
    return read_entry_from_buf(cursor);
}

int zelda64_set_dma_table_entry(uint8_t *dma_data, size_t size, uint32_t index, zelda64_dma_entry_t entry) {
    assert(dma_data != NULL);
    assert(index * 16 + 16 <= size);
    write_entry_to_buf(dma_data + 16 * index, entry);
    return 0;
}
