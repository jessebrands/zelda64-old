#include <assert.h>
#include <string.h>

#include "dma.h"
#include "util.h"

static inline zelda64_dma_entry_t read_entry_from_buf(uint8_t const *buf) {
    return (zelda64_dma_entry_t) {
            u32_from_buf(buf),
            u32_from_buf(buf + 4),
            u32_from_buf(buf + 8),
            u32_from_buf(buf + 12),
    };
}

static inline void write_entry_to_buf(uint8_t *buf, zelda64_dma_entry_t entry) {
    u32_to_buf(entry.v_start, buf);
    u32_to_buf(entry.v_end, buf+4);
    u32_to_buf(entry.p_start, buf+8);
    u32_to_buf(entry.p_end, buf+12);
}

static uint8_t const needle[] = {
        0x00, 0x00, 0x00, 0x00, // v_start1
        0x00, 0x00, 0x10, 0x60, // v_end1
        0x00, 0x00, 0x00, 0x00, // p_start1
        0x00, 0x00, 0x00, 0x00, // p_end1
        0x00, 0x00, 0x10, 0x60, // v_start2
};

int zelda64_find_dma_table_offset(uint8_t const *buffer, uint64_t buffer_length, uint64_t *out) {
    assert(buffer != NULL);
    assert(out != NULL);
    for (uint64_t offset = 0; offset + sizeof(needle) < buffer_length; offset += 4) {
        uint8_t const *cursor = buffer + offset;
        if (memcmp(cursor, needle, sizeof(needle)) == 0) {
            *out = offset;
            return 0;
        }
    }
    return -1;
}

zelda64_dma_info_t zelda64_get_dma_table_information(uint8_t const *buffer, uint64_t dma_offset) {
    assert(buffer != NULL);
    uint8_t const *cursor = buffer + (dma_offset + 32);
    const zelda64_dma_entry_t entry = read_entry_from_buf(cursor);
    return (zelda64_dma_info_t) {
            entry.v_start,
            entry.v_end - entry.v_start,
            (entry.v_end - entry.v_start) / 16
    };
}

zelda64_dma_entry_t zelda64_get_dma_table_entry(uint8_t const *dma_data, uint32_t size, uint32_t index) {
    assert(dma_data != NULL);
    assert(index * 16 + 16 <= size);
    uint8_t const *cursor = dma_data + index * 16;
    return read_entry_from_buf(cursor);
}

int zelda64_set_dma_table_entry(uint8_t *dma_data, uint32_t size, uint32_t index, zelda64_dma_entry_t entry) {
    assert(dma_data != NULL);
    assert(index * 16 + 16 <= size);
    write_entry_to_buf(dma_data + 16 * index, entry);
    return 0;
}
