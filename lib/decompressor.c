#include "decompressor.h"
#include "yaz0.h"

int zelda64_decompress_rom(zelda64_decompressor_info_t info) {
    zelda64_dma_info_t dma_info = info.dma_info;
    for (uint_fast32_t i = 0; i < dma_info.entries; ++i) {
        zelda64_dma_entry_t entry = zelda64_get_dma_table_entry(info.dma_data, dma_info.size, i);
        if (entry.p_start >= 0x4000000 || entry.p_end == 0xFFFFFFFF) {
            continue;
        } else if (entry.p_end == 0) {
            uint32_t file_size = entry.v_end - entry.v_start;
            if (file_size > 0) {
                uint8_t *data = info.load_rom_data(entry.p_start, file_size, info.userdata);
                info.write_out(entry.v_start, file_size, data, info.userdata);
                info.unload_rom_data(data, info.userdata);
            }
        } else {
            uint32_t file_size = entry.p_end - entry.p_start;
            uint8_t *data = info.load_rom_data(entry.p_start, file_size, info.userdata);
            zelda64_yaz0_header_t header = zelda64_get_yaz0_header(data);
            if (!zelda64_is_valid_yaz0_header(header)) {
                info.unload_rom_data(data, info.userdata);
                return 1;
            }
            uint8_t *out_data = info.allocate(header.uncompressed_size * sizeof(uint8_t), info.userdata);
            zelda64_yaz0_decompress(out_data, header.uncompressed_size, data);
            info.write_out(entry.v_start, header.uncompressed_size, out_data, info.userdata);
            info.unload_rom_data(data, info.userdata);
            info.deallocate(out_data, header.uncompressed_size * sizeof(uint8_t), info.userdata);
        }
        entry.p_start = entry.v_start;
        entry.p_end = 0;
        // TODO: Write DMA data to out table.
    }
    return 0;
}
