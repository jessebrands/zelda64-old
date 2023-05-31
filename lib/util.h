#pragma once

#include <stdint.h>

// Reads an uint32_t from an uint8_t array.
// Internal use only, unsafe code. Only call if you are certain buf is 4 elements or larger!
static inline uint32_t u32_from_buf(uint8_t const *buf) {
    return (((uint32_t) buf[0]) << 24
            | ((uint32_t) buf[1]) << 16
            | ((uint32_t) buf[2]) << 8
            | ((uint32_t) buf[3]) << 0);
}

// Writes an uint32_t to an uint8_t array.
// Internal use only, unsafe code. Only call if you are certain buf is 4 elements or larger!
static inline void u32_to_buf(uint32_t n, uint8_t *buf) {
    buf[0] = (uint8_t) (n >> 24);
    buf[1] = (uint8_t) (n >> 16);
    buf[2] = (uint8_t) (n >> 8);
    buf[3] = (uint8_t) n;
}
