#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "decompress.h"

#define ZELDA64_DEFAULT_OUTFILE "out.z64"

typedef struct zelda64_file_read_writer {
    FILE *in_file;
    FILE *out_file;
} zelda64_file_read_writer_t;

zelda64_file_read_writer_t zelda64_file_read_writer_open(const char *restrict in_filename, const char *restrict out_filename) {
    FILE* in_file = fopen(in_filename, "rb");
    FILE* out_file = fopen(out_filename, "wb");
    return (zelda64_file_read_writer_t) {
        .in_file = in_file,
        .out_file = out_file,
    };
}

void zelda64_file_read_writer_close(zelda64_file_read_writer_t read_writer) {
    fclose(read_writer.in_file);
    fclose(read_writer.out_file);
}

uint8_t *file_read_rom_data(size_t size, size_t offset, void *userdata) {
    zelda64_file_read_writer_t *read_writer = (zelda64_file_read_writer_t *) userdata;
    fseek(read_writer->in_file, (long) offset, SEEK_SET);
    uint8_t *data = malloc(size);
    if (data == nullptr) {
        return nullptr;
    }
    fread(data, sizeof (uint8_t), size, read_writer->in_file);
    if (ferror(read_writer->in_file)) {
        free(data);
        return nullptr;
    }
    return data;
}

void file_close_rom_data(uint8_t *data, size_t size, void *userdata) {
    free(data);
}

void file_reserve_space(size_t size, void *userdata) {
    zelda64_file_read_writer_t *read_writer = (zelda64_file_read_writer_t *) userdata;
    fseek(read_writer->out_file, 0, SEEK_SET);
    char nothing = '\0';
    fwrite(&nothing, sizeof(char), size, read_writer->out_file);
}

void file_write_out(uint8_t *data, size_t size, size_t offset, void *userdata) {
    zelda64_file_read_writer_t *read_writer = (zelda64_file_read_writer_t *) userdata;
    fseek(read_writer->out_file, (long) offset, SEEK_SET);
    fwrite(data, sizeof(uint8_t), size, read_writer->out_file);
}

zelda64_decompress_rom_params_t decompress_params_from_file_read_writer(zelda64_file_read_writer_t *read_writer) {
    fseek(read_writer->in_file, 0, SEEK_SET);
    fseek(read_writer->in_file, 0, SEEK_END);
    long filesize = ftell(read_writer->in_file);
    return (zelda64_decompress_rom_params_t) {
        .read_rom_data = file_read_rom_data,
        .close_rom_data = file_close_rom_data,
        .reserve = file_reserve_space,
        .write_data = file_write_out,
        .block_size = 1024 * 16,
        .rom_size = filesize,
        .userdata = read_writer,
    };
}

enum operation_mode {
    ZELDA64_MODE_NONE = 0,
    ZELDA64_MODE_COMPRESS = 1,
    ZELDA64_MODE_DECOMPRESS = 2,
    ZELDA64_MODE_PATCH = 4,
};

typedef struct zelda64_options {
    const char *in_filename;
    const char *out_filename;
    const char *patch_filename;
    enum operation_mode mode;
    bool show_help;
    bool show_version;
} zelda64_options_t;

void print_usage(FILE *stream) {
    assert(stream != NULL);
    fprintf(stream, "Usage: zelda64 [-hvcx] [-p patch_file] file [out_file]\n");
}

void print_version(void) {
    printf("zelda64 v0.1 - utility for manipulating Nintendo 64 Zelda ROMs\n");
    printf("\tby Jesse Gerard Brands <https://github.com/jessebrands/zelda64>\n");
}

void print_help() {
    print_usage(stdout);
    printf("Options:\n");
    printf("\t-h\n\t\tDisplay this information.\n");
    printf("\t-v\n\t\tDisplay version information.\n");
    printf("\t-c\n\t\tCompresses Nintendo 64 Zelda ROM file.\n");
    printf("\t-x\n\t\tDecompresses Nintendo 64 Zelda ROM file.\n");
    printf("\t-p=<patch_file>\n\t\tPatches a Nintendo 64 Zelda ROM with a ZPF patch file.\n");
}

void parse_command_line_opts(zelda64_options_t *opts, int argc, const char *const *argv) {
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
                        print_usage(stderr);
                        exit(EXIT_FAILURE);
                    }
                    break;
                default:
                    print_usage(stderr);
                    exit(EXIT_FAILURE);
            }
        } else {
            if (opts->in_filename == NULL) {
                opts->in_filename = arg;
            } else if (opts->out_filename == NULL) {
                opts->out_filename = arg;
            } else {
                print_usage(stderr);
                exit(EXIT_FAILURE);
            }
        }
    }
    if (opts->out_filename == NULL) {
        opts->out_filename = ZELDA64_DEFAULT_OUTFILE;
    }
    if (opts->mode == ZELDA64_MODE_NONE) {
        opts->mode = ZELDA64_MODE_DECOMPRESS;
    }
}

int main(int argc, char *argv[]) {
    zelda64_options_t opts = {};
    parse_command_line_opts(&opts, argc, (const char *const *) argv);
    if (opts.show_help) {
        print_help();
        return EXIT_SUCCESS;
    }
    if (opts.show_version) {
        print_version();
        return EXIT_SUCCESS;
    }
    if (opts.in_filename == NULL) {
        print_usage(stderr);
        return EXIT_FAILURE;
    }
    if (opts.mode & ZELDA64_MODE_DECOMPRESS) {
        zelda64_file_read_writer_t read_writer = zelda64_file_read_writer_open(opts.in_filename, opts.out_filename);
        zelda64_decompress_rom_params_t params = decompress_params_from_file_read_writer(&read_writer);
        zelda64_decompress_rom(params, zelda64_default_allocator());
        // TODO: Recalculate ROM Checksum here.
        zelda64_file_read_writer_close(read_writer);
    }
    if (opts.mode & ZELDA64_MODE_COMPRESS) {

    }
    return EXIT_SUCCESS;
}
