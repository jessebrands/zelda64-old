#include <zelda64/decompressor.h>
#include <zelda64/dma.h>
#include <zelda64/yaz0.h>

int zelda64_decompress_rom(zelda64_decompressor_info_t info) {
    zelda64_dma_info_t dma_info = info.dma_info;
    uint8_t *dma_out = info.allocate(dma_info.size, info.userdata);
    for (int_fast32_t i = 0; i < dma_info.entries; ++i) {
        zelda64_dma_entry_t entry = zelda64_get_dma_table_entry(info.dma_data, dma_info.size, i);
        zelda64_decompressor_action_t action = zelda64_get_decompressor_action(entry);
        switch (action) {
            case ZELDA64_ACTION_SKIP: {
                continue;
            }
            case ZELDA64_ACTION_COPY: {
                size_t file_size = zelda64_get_file_size(entry, action);
                if (file_size > 0) {
                    uint8_t *data = info.load_rom_data(entry.p_start, file_size, info.userdata);
                    info.write_out(entry.v_start, file_size, data, info.userdata);
                    info.unload_rom_data(data, info.userdata);
                }
                break;
            }
            case ZELDA64_ACTION_DECOMPRESS: {
                size_t file_size = zelda64_get_file_size(entry, action);
                uint8_t *data = info.load_rom_data(entry.p_start, file_size, info.userdata);
                zelda64_yaz0_header_t header = zelda64_get_yaz0_header(data, file_size);
                if (!zelda64_is_valid_yaz0_header(header)) {
                    info.unload_rom_data(data, info.userdata);
                    info.deallocate(dma_out, dma_info.size, info.userdata);
                    return 1;
                }
                uint8_t *out_data = info.allocate(header.uncompressed_size * sizeof(uint8_t), info.userdata);
                zelda64_yaz0_decompress(out_data, header.uncompressed_size, data);
                info.write_out(entry.v_start, header.uncompressed_size, out_data, info.userdata);
                if (info.unload_rom_data != NULL) {
                    info.unload_rom_data(data, info.userdata);
                }
                info.deallocate(out_data, header.uncompressed_size * sizeof(uint8_t), info.userdata);
                break;
            }
        }
        entry.p_start = entry.v_start;
        entry.p_end = 0;
        zelda64_set_dma_table_entry(dma_out, dma_info.size, i, entry);
    }
    info.write_out(dma_info.offset, dma_info.size, dma_out, info.userdata);
    info.deallocate(dma_out, dma_info.size, info.userdata);
    return 0;
}
