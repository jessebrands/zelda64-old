#include <zelda64/yaz0.h>
#include <assert.h>
#include <stdio.h>

#include "util.h"

#define YAZ0_MAX_LENGTH 0x111

zelda64_yaz0_header_t zelda64_get_yaz0_header(const uint8_t *buf, size_t size) {
    assert(size >= 16);
    zelda64_yaz0_header_t header = {
            .magic = {0},
            .uncompressed_size = u32_from_buf(buf + 4),
            .alignment = u32_from_buf(buf + 8),
    };
    memcpy(header.magic, buf, sizeof(header.magic));
    return header;
}

void zelda64_yaz0_decompress(uint8_t *dest, size_t dest_size, const uint8_t *src) {
    uint64_t src_index = 16; // skip header
    uint64_t dest_index = 0;
    uint8_t count = 0;
    uint8_t cb;
    while (dest_index < dest_size) {
        if (count == 0) {
            cb = src[src_index++];
            count = 8;
        }
        // Check the 7th bit, if it's a 1 we can just copy a byte. :-)
        if (cb & 0x80) {
            dest[dest_index++] = src[src_index++];
        } else {
            uint8_t b1 = src[src_index++];
            uint8_t b2 = src[src_index++];
            uint32_t distance = (((b1 & 0xF) << 8) | b2) + 1;
            uint32_t cursor = dest_index - distance;
            uint32_t bytes = b1 >> 4;
            if (bytes == 0) {
                bytes = src[src_index++] + 0x12;
            } else {
                bytes += 2;
            }
            for (uint32_t n = 0; n < bytes; ++n) {
                dest[dest_index++] = dest[cursor++];
            }
        }
        cb = cb << 1;
        count--;
    }
}

void yaz0_search(const uint8_t *src, size_t src_size, int pos, int max_length, int search_range,
                 int *restrict found, int *restrict found_length) {
    int f = 0;
    int f_len = 1;
    if (pos + 2 < src_size) {
        // Look back as far as the range allows us.
        int search = pos - search_range;
        if (search < 0) {
            search = 0;
        }
        // Calculate our end position.
        size_t end = pos + max_length;
        if (end > src_size) {
            end = src_size;
        }
        // Start searching.
        uint8_t needle = src[pos];
        while (search < pos) {
            // First we'll need to seek a matching byte.
            bool match_found = false;
            for (int i = search; i < pos; ++i) {
                if (src[i] == needle) {
                    search = i;
                    match_found = true;
                    break;
                }
            }
            if (!match_found) {
                break;
            }
            int p1 = search + 1;
            int p2 = pos + 1;
            while (p2 < end && src[p1] == src[p2]) {
                ++p1;
                ++p2;
            }
            int len = p2 - pos;
            if (f_len < len) {
                f_len = len;
                f = search;
                if (f_len == max_length) {
                    break;
                }
            }
            ++search;
        }
    }
    *found = f;
    *found_length = f_len;
}

void calculate_prefix_table(const uint8_t *restrict needle, size_t size, int *restrict table) {
    int len = 0;
    table[0] = 0;
    int i = 1;
    while (i < size) {
        if (needle[i] == needle[len]) {
            ++len;
            table[i] = len;
            ++i;
        } else {
            if (len != 0) {
                len = table[len - 1];
            } else {
                table[i] = 0;
                ++i;
            }
        }
    }
    return;
}

void find_longest_match(const uint8_t *data, size_t data_size,
                        const uint8_t *needle, size_t needle_size,
                        int *restrict match_start, int *restrict match_size) {
    // Create our prefix table first.
    int table[needle_size];
    calculate_prefix_table(needle, needle_size, table);

    int i = 0;
    int j = 0;
    int longest_start = 0;
    int longest_match = 0;
    while ((data_size - i) >= (needle_size - j)) {
        if (data[i] == needle[j]) {
            j++;
            i++;
        }
        // If our match is the same size as our needle, we got a full match, stop, this is as good as it'll get.
        if (j == needle_size) {
            longest_start = i - j;
            longest_match = j;
            break;
        }
            // We failed to match after X characters.
        else if (i < data_size && data[i] != needle[j]) {
            if (j != 0) {
                if (j > longest_match && j > 2) {
                    longest_start = i - j;
                    longest_match = j;
                }
                j = table[j - 1];
            } else {
                ++i;
            }
        }
    }
    *match_start = longest_start;
    *match_size = longest_match;
}

int zelda64_yaz0_compress_group(const uint8_t *restrict src, size_t src_size, int src_pos, int level,
                                zelda64_yaz0_data_group_t *restrict out) {
    int search_range = 0;
    // The level controls the search range:
    if (level >= 0 && level <= 9) {
        search_range = 0x10e0 * level / 9 - 0x0e0;
    }
    // Read over the source array.
    if (src_pos < src_size) {
        // Yaz0 compresses data into groups, a 1-byte header with 8 chunks of variable size, each bit in the header
        // Indicates the size of each chunk. If a bit is high (1), that means the chunk is one byte and should be copied
        // directly during decompression. If a bit is low (0), then the byte specifies the location in the compressed
        // data that must be read and copied. This is basically RLE style compression, slow but effective.
        //
        // Therefore, the compression level sets the search range. A level of 0 creates a search range of 0, therefore
        // all bits in the header will be set to 1 and the chunks will each be a byte long. This means that there is
        // no compression at all; and in fact the destination buffer will be larger than the source due to the Yaz0
        // metadata that will be attached to the uncompressed data and the 16-byte Yaz0 header.
        size_t group_pos = 0;
        uint8_t bitmask = 0x80;
        for (int i = 0; i < 8 && src_pos < src_size; ++i) {
            int found = 0;
            int found_length = 0;
            if (search_range > 0) {
                /*
                // Calculate look back position.
                int search = src_pos - search_range;
                if (search < 0) {
                    search = 0;
                }
                int search_end = search_range;
                if (search_end > src_size) {
                    search_end = src_size;
                }
                if (search_end > src_pos) {
                    search_end = src_pos;
                }
                // The size of the needle (ideally we find this!)
                size_t needle_size = YAZ0_MAX_LENGTH;
                if (needle_size > src_size - src_pos) {
                    needle_size = src_size - src_pos;
                }
                // Here, we now use our own algorithm!
                find_longest_match(src + search, search_end, src + src_pos, needle_size,
                                   &found, &found_length);
                found = search + found;
                */
                yaz0_search(src, src_size, src_pos, YAZ0_MAX_LENGTH, search_range, &found, &found_length);
            }
            if (found_length > 2) {
                int delta = src_pos - found - 1;
                if (found_length < 0x12) {
                    out->chunks[group_pos++] = ((found_length - 2) << 4) | (delta >> 8) ;
                    out->chunks[group_pos++] = (delta & 0xFF);
                } else {
                    out->chunks[group_pos++] = delta >> 8;
                    out->chunks[group_pos++] = delta & 0xFF;
                    out->chunks[group_pos++] = (found_length - 0x12) & 0xFF;
                }
                src_pos += found_length;
            } else {
                // This means that there was no found match, so we should copy the byte directly.
                out->chunks[group_pos++] = src[src_pos++];
                out->header |= bitmask;
            }
            bitmask = bitmask >> 1;
        }
        out->length = group_pos;
        return src_pos;
    }
    return 0;
}
