#pragma once

#include <stdint.h>

/**
 * Reads an uint32_t from an uint8_t array.
 * @param buf The buffer to read from.
 * @return The unsigned 32-bit integer.
 * @note This function is not bounds checked! Make sure to only call on a buffer of sufficient size.
 * @internal
 */
static inline uint32_t u32_from_buf(const uint8_t *buf) {
    return (((uint32_t) buf[0]) << 24
            | ((uint32_t) buf[1]) << 16
            | ((uint32_t) buf[2]) << 8
            | ((uint32_t) buf[3]) << 0);
}

static inline uint32_t u24_from_buf(const uint8_t *buf) {
    return (((uint32_t) buf[0]) << 16
            | ((uint32_t) buf[1]) << 8
            | ((uint32_t) buf[2]) << 0);
}

/**
 * Writes an uint32_t to an uint8_t array.
 * @param n The number to write.
 * @param buf The buffer to write to.
 * @note This function performs no bounds checking. Make sure to only call with a valid buffer.
 * @internal
 */
static inline void u32_to_buf(uint32_t n, uint8_t *buf) {
    buf[0] = (uint8_t) (n >> 24);
    buf[1] = (uint8_t) (n >> 16);
    buf[2] = (uint8_t) (n >> 8);
    buf[3] = (uint8_t) n;
}
