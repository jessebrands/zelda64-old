#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum zelda64_result {
    ZELDA64_OK = 0,
    ZELDA64_ERROR_INVALID_DATA = 1,
} zelda64_result_t;

typedef void *(zelda64_alloc_func_t)(size_t count, size_t size, void *userdata);
typedef void (zelda64_free_func_t)(void *data, void *userdata);

typedef struct zelda64_allocator {
    zelda64_alloc_func_t *alloc;
    zelda64_free_func_t *free;
    void *userdata;
} zelda64_allocator_t;

static inline void *zelda64_alloc(size_t count, size_t size, void *userdata) {
    return calloc(count, size);
}

static inline void zelda64_free(void *data, void *userdata) {
    free(data);
}

static inline zelda64_allocator_t zelda64_default_allocator() {
    return (zelda64_allocator_t) {
            .alloc = zelda64_alloc,
            .free = zelda64_free,
    };
}

typedef void *(zelda64_read_data_func_t)(size_t size, size_t offset, void *userdata);
typedef void (zelda64_close_data_func_t)(void *data, size_t size, void *userdata);
typedef void (zelda64_write_data_func_t)(void *data, size_t size, size_t offset, void *userdata);
