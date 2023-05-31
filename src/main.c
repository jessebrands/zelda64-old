#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "../lib/dma.h"
#include "../lib/yaz0.h"
#include "../lib/rom.h"
#include "../lib/decompressor.h"

#define DEFAULT_OUTFILE "out.z64"

#define CHUNK_SIZE 4096

enum operation_mode {
    ZELDA64_MODE_NONE = 0,
    ZELDA64_MODE_COMPRESS = 1,
    ZELDA64_MODE_DECOMPRESS = 2,
    ZELDA64_MODE_PATCH = 3,
};

typedef struct zelda64_options {
    char const *in_filename;
    char const *out_filename;
    char const *patch_filename;
    enum operation_mode mode;
    bool show_help;
    bool show_version;
} zelda64_options_t;

void print_usage(FILE *stream, char const *program_name) {
    assert(stream != NULL);
    assert(program_name != NULL);
    fprintf(stream, "Usage: zelda64 [-hvcx] [-p patch_file] file [out_file]\n");
}

void print_version(void) {
    printf("zelda64 v0.1 - utility for manipulating Nintendo 64 Zelda ROMs\n");
    printf("\tby Jesse Gerard Brands <https://github.com/jessebrands/zelda64>\n");
}

void print_help(char const *program_name) {
    print_usage(stdout, program_name);
    printf("Options:\n");
    printf("\t-h\n\t\tDisplay this information.\n");
    printf("\t-v\n\t\tDisplay version information.\n");
    printf("\t-c\n\t\tCompresses Nintendo 64 Zelda ROM file.\n");
    printf("\t-x\n\t\tDecompresses Nintendo 64 Zelda ROM file.\n");
    printf("\t-p=<patch_file>\n\t\tPatches a Nintendo 64 Zelda ROM with a ZPF patch file.\n");
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
                case 'c':
                    opts->mode = ZELDA64_MODE_COMPRESS;
                    break;
                case 'x':
                    opts->mode = ZELDA64_MODE_DECOMPRESS;
                    break;
                case 'p':
                    opts->mode = ZELDA64_MODE_PATCH;
                    if (i + 1 < argc) {
                        opts->patch_filename = argv[++i];
                    } else {
                        print_usage(stderr, argv[0]);
                        exit(EXIT_FAILURE);
                    }
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
    if (opts->out_filename == NULL) {
        opts->out_filename = DEFAULT_OUTFILE;
    }
    if (opts->mode == ZELDA64_MODE_NONE) {
        opts->mode = ZELDA64_MODE_DECOMPRESS;
    }
}

typedef struct zelda64_userdata {
    FILE *in_file;
    FILE *out_file;
} zelda64_userdata_t;

uint8_t *allocate(size_t size, void *userdata) {
    return calloc(sizeof(uint8_t), size);
}

void deallocate(uint8_t *ptr, size_t size, void *userdata) {
    free(ptr);
}

uint8_t *load_rom_data(uint32_t offset, uint32_t size, void *userdata) {
    if (userdata == NULL) {
        return NULL;
    }
    zelda64_userdata_t *d = (zelda64_userdata_t *) userdata;
    if (fseek(d->in_file, offset, SEEK_SET)) {
        return NULL;
    }
    uint8_t *data = allocate(size, userdata);
    if (!fread(data, sizeof(uint8_t), size, d->in_file)) {
        deallocate(data, size, userdata);
        return NULL;
    }
    return data;
}

void unload_rom_data(uint8_t *data, void *userdata) {
    free(data);
}

void write_out(uint64_t offset, size_t size, uint8_t *data, void *userdata) {
    if (userdata == NULL) return;
    zelda64_userdata_t *d = (zelda64_userdata_t *) userdata;
    if (fseek(d->out_file, offset, SEEK_SET)) {
        return;
    }
    fwrite(data, sizeof(uint8_t), size, d->out_file);
}

int main(int argc, char **argv) {
    zelda64_options_t opts = {0};
    parse_command_line_opts(&opts, argc, (char const *const *) argv);
    if (opts.show_help) {
        print_help(argv[0]);
        return EXIT_SUCCESS;
    }
    if (opts.show_version) {
        print_version();
        return EXIT_SUCCESS;
    }
    if (opts.in_filename == NULL) {
        print_usage(stderr, argv[0]);
        return EXIT_FAILURE;
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
    zelda64_userdata_t userdata = {
            .in_file = in_file,
            .out_file = out_file,
    };
    zelda64_decompress_rom((zelda64_decompressor_info_t) {
            .dma_info = info,
            .dma_data = dma_data,
            .allocate = allocate,
            .deallocate = deallocate,
            .load_rom_data = load_rom_data,
            .unload_rom_data = unload_rom_data,
            .write_out = write_out,
            .userdata = &userdata,
    });
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
        fprintf(stderr, "failed to read MAKEROM from output ROM\n");
        return EXIT_FAILURE;
    }
    zelda64_rom_header_t rom_header = {0};
    zelda64_read_rom_header_from_buffer(&rom_header, makerom, 0x101000);
    uint32_t cic = zelda64_calculate_rom_cic(rom_header.bootcode, 4032);
    uint32_t crc1 = 0, crc2 = 0;
    zelda64_calculate_rom_checksum(makerom, 0x101000, cic, &crc1, &crc2);
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
