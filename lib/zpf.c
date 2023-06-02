#include <assert.h>

#include <zelda64/zpf.h>

#include "util.h"

int zelda64_zpf1_read_header(zelda64_zpf1_header_t *out, const uint8_t *data, size_t size) {
    assert(out != NULL);
    assert(data != NULL);
    if (size >= 21) {
        return 1;
    }
    memcpy(out->magic, data, sizeof ZPF1_MAGIC);
    out->dma_offset = u32_from_buf(data + 5);
    out->xor_start = u32_from_buf(data + 9);
    out->xor_end = u32_from_buf(data + 13);
    out->xor_address = u32_from_buf(data + 17);
    return 0;
}
