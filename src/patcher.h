#pragma once

typedef struct zelda64_decompress_zpf_info {
    FILE *patch_file;
    uint8_t **out_buffer;
    size_t* buffer_size;
    void *(*realloc_func)(void *, size_t new_size);
} zelda64_decompress_zpf_info_t;

int decompress_zpf_file(zelda64_decompress_zpf_info_t info);
