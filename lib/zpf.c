#include <assert.h>

#include <zelda64/zpf.h>

#include "util.h"

int zelda64_zpf1_read_header(zelda64_zpf1_header_t *out, const uint8_t *data, size_t size) {
    assert(out != NULL);
    assert(data != NULL);
    if (size < 21) {
        return 1;
    }
    memcpy(out->magic, data, sizeof ZPF1_MAGIC);
    out->dma_offset = u32_from_buf(data + 5);
    out->xor_start = u32_from_buf(data + 9);
    out->xor_end = u32_from_buf(data + 13);
    out->xor_address = u32_from_buf(data + 17);
    return 0;
}

bool zelda64_zpf_get_next_dma_entry(zelda64_zpf_dma_reader_t *reader, zelda64_zpf_dma_entry_t *dma_entry) {
    size_t offset = reader->offset + reader->index * 13;
    if (offset + 13 > reader->size) {
        return false; // End of data.
    }
    uint16_t dma_index = u16_from_buf(reader->data + offset);
    if (dma_index == 0xFFFF) {
        return false; // End of table.
    }
    dma_entry->index = dma_index;
    dma_entry->from_file = u32_from_buf(reader->data + offset + 2);
    dma_entry->offset = u32_from_buf(reader->data + offset + 6);
    dma_entry->size = u24_from_buf(reader->data + offset + 10);
    reader->index++;
    return true;
}

