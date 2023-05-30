#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "../lib/dma.h"
#include "../lib/yaz0.h"
#include "../lib/crc32.h"
#include "../lib/rom.h"

#define DEFAULT_INFILE "ZOOTENC.z64"
#define DEFAULT_OUTFILE "ZOOTDEC.z64"

#define CHUNK_SIZE 4096

typedef struct zelda64_options {
    char const *in_filename;
    char const *out_filename;
    bool show_help;
    bool show_version;
} zelda64_options_t;

void print_usage(FILE *stream, char const *const program_name) {
    assert(stream != NULL);
    assert(program_name != NULL);
    fprintf(stream, "Usage: %s [-hv] file [out_file]\n", program_name);
}

void print_version(void) {
    printf("zelda64 v0.1 - utility for manipulating Nintendo 64 Zelda ROMs\n");
    printf("\tby Jesse Gerard Brands <https://github.com/jessebrands/zelda64>\n");
}

void parse_command_line_opts(zelda64_options_t *opts, int argc, char const *const *argv) {
    assert(opts != NULL);
    for (int i = 1; i < argc; i++) {
        const char *arg = argv[i];
        if (strlen(arg) == 2 && arg[0] == '-') {
            switch (arg[1]) {
                case 'v':
                    opts->show_version = true;
                    break;
                case 'h':
                    opts->show_help = true;
                    break;
                default:
                    print_usage(stderr, argv[0]);
                    exit(EXIT_FAILURE);
            }
        } else {
            if (opts->in_filename == NULL) {
                opts->in_filename = arg;
            } else if (opts->out_filename == NULL) {
                opts->out_filename = arg;
            } else {
                print_usage(stderr, argv[0]);
                exit(EXIT_FAILURE);
            }
        }
    }
    // If in_file and/or out_file are blank, assume their defaults.
    if (opts->in_filename == NULL) {
        opts->in_filename = DEFAULT_INFILE;
    }
    if (opts->out_filename == NULL) {
        opts->out_filename = DEFAULT_OUTFILE;
    }
}

int main(int argc, char **argv) {
    zelda64_options_t opts = {0};
    parse_command_line_opts(&opts, argc, (char const *const *) argv);
    if (opts.show_help) {
        print_usage(stdout, argv[0]);
        return EXIT_SUCCESS;
    }
    if (opts.show_version) {
        print_version();
        return EXIT_SUCCESS;
    }
    // open the ROM file if possible
    FILE *in_file = fopen(opts.in_filename, "rb");
    if (in_file == NULL) {
        fprintf(stderr, "failed to open '%s', check if the file exists or may already be in use\n", opts.in_filename);
        return EXIT_FAILURE;
    }
    uint8_t buffer[CHUNK_SIZE];
    // now, let us seek the DMA table
    uint64_t dma_offset = 0;
    uint64_t offset = 0;
    while (!ferror(in_file)) {
        if (!fread(buffer, sizeof(uint8_t), CHUNK_SIZE, in_file)) {
            if (feof(in_file)) {
                break;
            }
            fprintf(stderr,
                    "failed to read ROM data from '%s', please check if this a valid, compressed N64 Zelda ROM\n",
                    opts.in_filename);
            return EXIT_FAILURE;
        }
        if (!zelda64_find_dma_table_offset(buffer, CHUNK_SIZE, &dma_offset)) {
            break;
        }
        offset += CHUNK_SIZE;
    }
    // Grab information about the DMA table and load the whole thing into memory.
    zelda64_dma_info_t const info = zelda64_get_dma_table_information(buffer, dma_offset);
    if (fseek(in_file, info.offset, SEEK_SET)) {
        fprintf(stderr, "failed to read ROM file '%s'\n", opts.in_filename);
        return EXIT_FAILURE;
    }
    // Allocate memory to hold the DMA table.
    uint8_t *dma_data = malloc(info.size * sizeof(uint8_t));
    if (!fread(dma_data, sizeof(uint8_t), info.size, in_file)) {
        fprintf(stderr, "failed to read DMA table from ROM file '%s'\n", opts.in_filename);
        free(dma_data);
        return EXIT_FAILURE;
    }
    // Open up the output ROM file
    // Allocate memory to hold the new DMA table.
    FILE *out_file = fopen(opts.out_filename, "wb+");
    if (out_file == NULL) {
        fprintf(stderr, "failed to open '%s', it may already be in use\n", opts.out_filename);
        free(dma_data);
        return EXIT_FAILURE;
    }
    {
        uint8_t zeroes[8192] = {0};
        for (int n = 0; n < 8192; n++) { // 8192^2 = 64 MB
            fwrite(zeroes, sizeof(uint8_t), sizeof zeroes, out_file);
        }
        fflush(out_file);
        fseek(out_file, 0, SEEK_SET);
    }
    uint8_t *dma_out = calloc(info.size, sizeof(uint8_t));
    for (uint32_t i = 0; i < info.entries; ++i) {
        zelda64_dma_entry_t entry = zelda64_get_dma_table_entry(dma_data, info.size, i);
        if (entry.p_start >= 0x4000000 || entry.p_end == 0xFFFFFFFF) {
            // Not sure if this can even happen, but if the file is located OOB we should just skip it.
            continue;
        } else if (entry.p_end == 0x0) {
            // No decompression necessary, just copy the file from ROM.
            uint32_t ent_size = entry.v_end - entry.v_start;
            if (ent_size != 0) {
                fseek(in_file, entry.p_start, SEEK_SET);
                uint8_t *data = calloc(ent_size, sizeof(uint8_t));
                if (!fread(data, sizeof(uint8_t), ent_size, in_file)) {
                    fprintf(stderr, "failed to read file to memory\n");
                    return EXIT_FAILURE;
                }
                // Write the data to output ROM.
                fseek(out_file, entry.v_start, SEEK_SET);
                fwrite(data, sizeof(uint8_t), ent_size, out_file);
                free(data);
            }
        } else {
            // Decompress the file.
            uint32_t ent_size = entry.p_end - entry.p_start;
            if (fseek(in_file, entry.p_start, SEEK_SET)) {
                fprintf(stderr, "failed to read ROM file '%s'\n", opts.in_filename);
                return EXIT_FAILURE;
            }
            uint8_t *compressed_data = calloc(ent_size, sizeof(uint8_t));
            if (!fread(compressed_data, sizeof(uint8_t), ent_size, in_file)) {
                fprintf(stderr, "failed to read file to memory\n");
                return EXIT_FAILURE;
            }
            zelda64_yaz0_header_t yaz0_header = zelda64_get_yaz0_header(compressed_data);
            if (!zelda64_is_valid_yaz0_header(yaz0_header)) {
                fprintf(stderr, "cannot decompress data, not valid Yaz0 compressed data\n");
                return EXIT_FAILURE;
            }
            uint8_t *decompressed_data = calloc(yaz0_header.uncompressed_size, sizeof(uint8_t));
            zelda64_yaz0_decompress(decompressed_data, yaz0_header.uncompressed_size, compressed_data);
            free(compressed_data);
            // Write our decompressed data to the output ROM.
            fseek(out_file, entry.v_start, SEEK_SET);
            fwrite(decompressed_data, sizeof(uint8_t), yaz0_header.uncompressed_size, out_file);
            free(decompressed_data);
        }
        // decompressed data always has p_end == 0 and p_start == v_start
        entry.p_start = entry.v_start;
        entry.p_end = 0x0;
        zelda64_set_dma_table_entry(dma_out, info.size, i, entry);
    }
    // Write out the new DMA table.
    fseek(out_file, info.offset, SEEK_SET);
    fwrite(dma_out, sizeof(uint8_t), info.size, out_file);
    free(dma_out);
    free(dma_data);
    // Recalculate the CRC-32 checksum of the output ROM.
    fflush(out_file);
    if (fseek(out_file, 0, SEEK_SET)) {
        fprintf(stderr, "failed to read output ROM\n");
        return EXIT_FAILURE;
    }
    uint8_t *makerom = calloc(0x101000, sizeof(uint8_t));
    if (!fread(makerom, sizeof(uint8_t), 0x101000, out_file) && ferror(out_file)) {
        fprintf(stderr, "failed to read MAKEROM from output ROM: %s\n",
                strerror(errno));
        return EXIT_FAILURE;
    }
    zelda64_rom_header_t rom_header = {0};
    zelda64_read_rom_header_from_buffer(&rom_header, makerom, 0x101000);
    uint32_t cic = zelda64_calculate_rom_cic(rom_header.bootcode, 4032);
    uint32_t crc1 = 0, crc2 = 0;
    zelda64_calculate_rom_checksum(makerom, 0x101000, cic, &crc1, &crc2);
    printf("CIC for the output ROM is: %d\n", cic);
    printf("Checksum for this ROM is 0x%08X%08X\n", crc1, crc2);
    uint8_t crc[8] = {0};
    u32_to_buf(crc1, crc);
    u32_to_buf(crc2, crc + 4);
    fseek(out_file, 16, SEEK_SET);
    fwrite(crc, sizeof(uint8_t), sizeof crc, out_file);
    // Cleanup.
    fclose(in_file);
    fclose(out_file);
    return EXIT_SUCCESS;
}
