#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define ZELDA64_DEFAULT_OUTFILE "out.z64"

enum operation_mode {
    ZELDA64_MODE_NONE = 0,
    ZELDA64_MODE_COMPRESS = 1,
    ZELDA64_MODE_DECOMPRESS = 2,
    ZELDA64_MODE_PATCH = 3,
};

typedef struct zelda64_options {
    const char *in_filename;
    const char *out_filename;
    const char *patch_filename;
    enum operation_mode mode;
    bool show_help;
    bool show_version;
} zelda64_options_t;

void print_usage(FILE *stream, const char *program_name) {
    assert(stream != NULL);
    assert(program_name != NULL);
    fprintf(stream, "Usage: zelda64 [-hvcx] [-p patch_file] file [out_file]\n");
}

void print_version(void) {
    printf("zelda64 v0.1 - utility for manipulating Nintendo 64 Zelda ROMs\n");
    printf("\tby Jesse Gerard Brands <https://github.com/jessebrands/zelda64>\n");
}

void print_help(const char *program_name) {
    print_usage(stdout, program_name);
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
    return EXIT_SUCCESS;
}
